#!/usr/bin/env python
#
# Copyright (c) 2014 Apple Inc. All rights reserved.
# Copyright (c) 2014 University of Washington. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.


import logging
import string
import re
from string import Template

from generate_objective_c import ObjCTypeCategory, ObjCGenerator, join_type_and_name
from generator import Generator
from generator_templates import GeneratorTemplates as Templates
from models import Frameworks

log = logging.getLogger('global')


class ObjectiveCConfigurationImplementationGenerator(Generator):
    def __init__(self, model, input_filepath):
        Generator.__init__(self, model, input_filepath)

    def output_filename(self):
        return '%sBackendDispatchers.mm' % ObjCGenerator.OBJC_PREFIX

    def domains_to_generate(self):
        return filter(ObjCGenerator.should_generate_domain_command_handler_filter(self.model()), Generator.domains_to_generate(self))

    def generate_output(self):
        secondary_headers = [
            '"%sInternal.h"' % ObjCGenerator.OBJC_PREFIX,
            '"%sEnumConversionHelpers.h"' % ObjCGenerator.OBJC_PREFIX,
            '<JavaScriptCore/InspectorFrontendChannel.h>',
            '<JavaScriptCore/InspectorValues.h>',
        ]

        header_args = {
            'primaryInclude': '"%sBackendDispatchers.h"' % ObjCGenerator.OBJC_PREFIX,
            'secondaryIncludes': '\n'.join(['#include %s' % header for header in secondary_headers]),
        }

        domains = self.domains_to_generate()
        sections = []
        sections.append(self.generate_license())
        sections.append(Template(Templates.ObjCBackendDispatcherImplementationPrelude).substitute(None, **header_args))
        sections.extend(map(self._generate_handler_implementation_for_domain, domains))
        sections.append(Template(Templates.ObjCBackendDispatcherImplementationPostlude).substitute(None, **header_args))
        return '\n\n'.join(sections)

    def _generate_handler_implementation_for_domain(self, domain):
        if not domain.commands:
            return ''

        command_declarations = []
        for command in domain.commands:
            command_declarations.append(self._generate_handler_implementation_for_command(domain, command))

        return '\n'.join(command_declarations)

    def _generate_handler_implementation_for_command(self, domain, command):
        lines = []
        parameters = ['long callId']
        for parameter in command.call_parameters:
            parameters.append('%s in_%s' % (Generator.type_string_for_unchecked_formal_in_parameter(parameter), parameter.parameter_name))

        command_args = {
            'domainName': domain.domain_name,
            'commandName': command.command_name,
            'parameters': ', '.join(parameters),
            'successCallback': self._generate_success_block_for_command(domain, command),
            'conversions': self._generate_conversions_for_command(domain, command),
            'invocation': self._generate_invocation_for_command(domain, command),
        }

        return self.wrap_with_guard_for_domain(domain, Template(Templates.ObjCBackendDispatcherHeaderDomainHandlerImplementation).substitute(None, **command_args))

    def _generate_success_block_for_command(self, domain, command):
        lines = []

        if command.return_parameters:
            success_block_parameters = []
            for parameter in command.return_parameters:
                objc_type = ObjCGenerator.objc_type_for_param(domain, command.command_name, parameter)
                var_name = ObjCGenerator.identifier_to_objc_identifier(parameter.parameter_name)
                success_block_parameters.append(join_type_and_name(objc_type, var_name))
            lines.append('    id successCallback = ^(%s) {' % ', '.join(success_block_parameters))
        else:
            lines.append('    id successCallback = ^{')

        if command.return_parameters:
            lines.append('        RefPtr<InspectorObject> resultObject = InspectorObject::create();')

            required_pointer_parameters = filter(lambda parameter: not parameter.is_optional and ObjCGenerator.is_type_objc_pointer_type(parameter.type), command.return_parameters)
            for parameter in required_pointer_parameters:
                var_name = ObjCGenerator.identifier_to_objc_identifier(parameter.parameter_name)
                lines.append('        THROW_EXCEPTION_FOR_REQUIRED_PARAMETER(%s, @"%s");' % (var_name, var_name))

            optional_pointer_parameters = filter(lambda parameter: parameter.is_optional and ObjCGenerator.is_type_objc_pointer_type(parameter.type), command.return_parameters)
            for parameter in optional_pointer_parameters:
                var_name = ObjCGenerator.identifier_to_objc_identifier(parameter.parameter_name)
                lines.append('        THROW_EXCEPTION_FOR_BAD_OPTIONAL_PARAMETER(%s, @"%s");' % (var_name, var_name))

            for parameter in command.return_parameters:
                keyed_set_method = Generator.keyed_set_method_for_type(parameter.type)
                var_name = ObjCGenerator.identifier_to_objc_identifier(parameter.parameter_name)
                var_expression = '*%s' % var_name if parameter.is_optional else var_name
                export_expression = ObjCGenerator.objc_protocol_export_expression_for_variable(parameter.type, var_expression)
                if not parameter.is_optional:
                    lines.append('        resultObject->%s(ASCIILiteral("%s"), %s);' % (keyed_set_method, parameter.parameter_name, export_expression))
                else:
                    lines.append('        if (%s)' % var_name)
                    lines.append('            resultObject->%s(ASCIILiteral("%s"), %s);' % (keyed_set_method, parameter.parameter_name, export_expression))
            lines.append('        backendDispatcher()->sendResponse(callId, resultObject.release(), String());')
        else:
            lines.append('        backendDispatcher()->sendResponse(callId, InspectorObject::create(), String());')

        lines.append('    };')
        return '\n'.join(lines)

    def _generate_conversions_for_command(self, domain, command):
        lines = []
        if command.call_parameters:
            lines.append('')

        for parameter in command.call_parameters:
            in_param_name = 'in_%s' % parameter.parameter_name
            objc_in_param_name = 'o_%s' % in_param_name
            objc_type = ObjCGenerator.objc_type_for_param(domain, command.command_name, parameter, False)
            in_param_optional_safe_name = '*%s' % in_param_name if parameter.is_optional else in_param_name
            import_expression = ObjCGenerator.objc_protocol_import_expression_for_parameter(in_param_optional_safe_name, domain, command.command_name, parameter)
            if not parameter.is_optional:
                lines.append('    %s = %s;' % (join_type_and_name(objc_type, objc_in_param_name), import_expression))
            else:
                lines.append('    %s;' % join_type_and_name(objc_type, objc_in_param_name))
                lines.append('    if (%s)' % in_param_name)
                lines.append('        %s = %s;' % (objc_in_param_name, import_expression))

        if lines:
            lines.append('')
        return '\n'.join(lines)

    def _generate_invocation_for_command(self, domain, command):
        pairs = []
        pairs.append('WithErrorCallback:errorCallback')
        pairs.append('successCallback:successCallback')
        for parameter in command.call_parameters:
            in_param_name = 'in_%s' % parameter.parameter_name
            objc_in_param_name = 'o_%s' % in_param_name
            if not parameter.is_optional:
                pairs.append('%s:%s' % (parameter.parameter_name, objc_in_param_name))
            else:
                optional_expression = '(%s ? &%s : nil)' % (in_param_name, objc_in_param_name)
                pairs.append('%s:%s' % (parameter.parameter_name, optional_expression))
        return '    [m_delegate %s%s];' % (command.command_name, ' '.join(pairs))
