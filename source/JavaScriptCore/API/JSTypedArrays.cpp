//
//  TypedArrays.cpp
//  JSTypedArrays.cpp
//
//  Created by Imanol Fernandez Gorostizaga on 26/04/13.
//  Copyright (c) 2013 Ideateca. All rights reserved.
//

//Constructors
#include "config.h"
#include "API/APICast.h"
#include "API/JSTypedArrays.h"
#include "JSArrayBuffer.h"
#include "JSFloat32Array.h"
#include "JSFloat64Array.h"
#include "JSInt8Array.h"
#include "JSInt16Array.h"
#include "JSInt32Array.h"
#include "JSUint8ClampedArray.h"
#include "JSUint8Array.h"
#include "JSUint16Array.h"
#include "JSUint32Array.h"

#include "TypedArrayInlines.h"

using namespace JSC;

template <typename ArrayType>JSObject * CreateTypedArray(JSC::ExecState* exec, size_t length) {
    return ArrayType::create(length)->wrap(exec, exec->lexicalGlobalObject());
}

static JSObject * CreateDataView(JSC::ExecState* exec, size_t length)
{
    return DataView::create(ArrayBuffer::create(length, 1))->wrap(exec, exec->lexicalGlobalObject());
}

/*
 * Makes an ArrayBufferView object with the desided type and element count
 */
JSObjectRef JSObjectMakeArrayBufferView(JSContextRef ctx, JSObjectArrayBufferViewType viewType, unsigned int numElements)
{
    ExecState * exec = toJS(ctx);
    
    JSObject * result;
    
    switch (viewType) {
        default:
        case kJSArrayBufferViewDataView: result = CreateDataView(exec, numElements); break;
        case kJSArrayBufferViewUint8: result = CreateTypedArray<Uint8Array>(exec, numElements); break;
        case kJSArrayBufferViewInt8: result = CreateTypedArray<Int8Array>(exec, numElements); break; break;
        case kJSArrayBufferViewUint8Clamped: result = CreateTypedArray<Uint8ClampedArray>(exec, numElements); break;
        case kJSArrayBufferViewInt16: result = CreateTypedArray<Int16Array>(exec, numElements); break;
        case kJSArrayBufferViewUint16: result = CreateTypedArray<Uint16Array>(exec, numElements); break;
        case kJSArrayBufferViewInt32: result = CreateTypedArray<Int32Array>(exec, numElements); break;
        case kJSArrayBufferViewUint32: result = CreateTypedArray<Uint32Array>(exec, numElements); break;
        case kJSArrayBufferViewFloat32: result = CreateTypedArray<Float32Array>(exec, numElements); break;
        case kJSArrayBufferViewFloat64: result = CreateTypedArray<Float64Array>(exec, numElements); break;
    }
    
    return toRef(result);
}

/*
 * Makes an ArrayBuffer object with the desided bytes count
 */
JSObjectRef JSObjectMakeArrayBuffer(JSContextRef ctx, unsigned int nBytes)
{
    ExecState * exec = toJS(ctx);
    RefPtr<ArrayBuffer> buffer = ArrayBuffer::create(nBytes, 1);
    if( !buffer ) {
        return NULL;
    }
    
    JSArrayBuffer* result = JSArrayBuffer::create(exec->vm(), exec->lexicalGlobalObject()->arrayBufferStructure(), buffer);
    return toRef(result);
}


//Type check

/*
 * Tests if the object supports get raw data methxods (it can be an ArrayBufferView or an ArrayBuffer);
 */
bool JSObjectIsTypedArray(JSContextRef ctx, JSObjectRef obj)
{
    (void)ctx;
    JSObject * object = toJS(obj);
    JSC::JSType type = object->type();
    return (type >=Int8ArrayType && type <= DataViewType) || object->inherits(JSArrayBuffer::info());
}

/*
 * Tests if the object is an ArrayBufferView object
 */
bool JSObjectIsArrayBufferView(JSContextRef ctx, JSObjectRef obj)
{
    (void)ctx;
    JSObject * object = toJS(obj);
    JSC::JSType type = object->type();
    return type >=Int8ArrayType && type <= DataViewType;
}

/*
 * Tests if the object is an ArrayBufferView object of the desired type
 */
bool JSObjectIsArrayBufferViewOfType(JSContextRef ctx, JSObjectRef obj, JSObjectArrayBufferViewType viewType)
{
    (void)ctx;
    JSObject * object = toJS(obj);
    JSC::JSType type = object->type();
    switch (viewType) {
        case kJSArrayBufferViewInt8: return type == Int8ArrayType;
        case kJSArrayBufferViewUint8: return type == Uint8ArrayType;
        case kJSArrayBufferViewUint8Clamped: return type == Uint8ClampedArrayType;
        case kJSArrayBufferViewInt16: return type == Int16ArrayType;
        case kJSArrayBufferViewUint16: return type == Uint16ArrayType;
        case kJSArrayBufferViewInt32: return type == Int32ArrayType;
        case kJSArrayBufferViewUint32: return type == Uint32ArrayType;
        case kJSArrayBufferViewFloat32: return type == Float32ArrayType;
        case kJSArrayBufferViewFloat64: return type == Float64ArrayType;
        case kJSArrayBufferViewDataView: return type == DataViewType;
    }
    return false;
}

/*
 * Get the type of elements in a typed array, or TYPE_DATAVIEW if a DataView
 */
bool JSObjectGetArrayBufferViewType(JSContextRef ctx, JSObjectRef obj, JSObjectArrayBufferViewType & viewType)
{
    (void)ctx;
    JSObject * object = toJS(obj);
    if (object->inherits(JSArrayBufferView::info()))
    {
        switch (object->type()) {
            case Int8ArrayType: viewType = kJSArrayBufferViewInt8; break;
            case Uint8ArrayType: viewType = kJSArrayBufferViewUint8; break;
            case Uint8ClampedArrayType: viewType = kJSArrayBufferViewUint8Clamped; break;
            case Int16ArrayType: viewType = kJSArrayBufferViewInt16; break;
            case Uint16ArrayType: viewType = kJSArrayBufferViewUint16; break;
            case Int32ArrayType: viewType = kJSArrayBufferViewInt32; break;
            case Uint32ArrayType: viewType = kJSArrayBufferViewUint32; break;
            case Float32ArrayType: viewType = kJSArrayBufferViewFloat32; break;
            case Float64ArrayType: viewType = kJSArrayBufferViewFloat64; break;
            case DataViewType: viewType = kJSArrayBufferViewDataView; break;
            default: return false;
        }
        return true;
    }
    return false;
}

/*
 * Tests if the object is an ArrayBuffer object
 */
bool JSObjectIsArrayBuffer(JSContextRef ctx, JSObjectRef obj)
{
    (void)ctx;
    JSObject * object = toJS(obj);
    return object->inherits(JSArrayBuffer::info());
}

/*
 * Tests if the object is a normal Array
 */
bool JSObjectIsUntypedArray(JSContextRef ctx, JSObjectRef obj)
{
    (void)ctx;
    JSObject * object = toJS(obj);
    return object->inherits(JSArray::info());
}

/*
 * Gets the length of an untyped array
 */
unsigned int JSObjectGetUntypedArrayLength(JSContextRef ctx, JSObjectRef obj)
{
    (void)ctx;
    JSObject * object = toJS(obj);
    if (object->inherits(JSArray::info()))
    {
        JSArray * array = reinterpret_cast<JSArray*>(object);
        return array->length();
    }
    return 0;
}

//Get Data

/*
 * Gets the raw data of any typed array object (ArrayBuffer or ArrayBufferView)
 */
bool JSObjectGetTypedArrayData(JSContextRef ctx, JSObjectRef obj, unsigned int & numBytes, uint8_t * & data)
{
    (void)ctx;
    JSObject * object = toJS(obj);
    if (JSArrayBufferView * view = jsDynamicCast<JSArrayBufferView*>(object)) {
        data = reinterpret_cast<uint8_t*>(view->impl()->baseAddress());
        numBytes = view->impl()->byteLength();
        return true;
    }
    else if (JSArrayBuffer * buffer = jsDynamicCast<JSArrayBuffer*>(object)) {
        data = reinterpret_cast<uint8_t*>(buffer->impl()->data());
        numBytes = buffer->impl()->byteLength();
        return true;
    }
    return false;
    
}

template<typename ArrayBufferClass, typename ImplClass, typename T> bool GetArrayBufferViewDataHelper(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, T* & data )
{
    (void)ctx;
    JSObject * object = toJS(obj);
    if (ArrayBufferClass * view = jsDynamicCast<ArrayBufferClass *>(object))
    {
        data = reinterpret_cast<T*>(view->impl()->baseAddress());
        numElements = view->impl()->byteLength() / sizeof(T);
        return true;
    }
    return false;
}

//Explicit Getters
bool JSObjectGetInt8ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, int8_t* & data)
{
    return GetArrayBufferViewDataHelper<JSInt8Array, Int8Array>(ctx, obj, numElements, data);
}

bool JSObjectGetUint8ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, uint8_t* & data)
{
    return GetArrayBufferViewDataHelper<JSUint8Array, Uint8Array>(ctx, obj, numElements, data);
}

bool JSObjectGetUint8ClampedArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, uint8_t* & data)
{
    return GetArrayBufferViewDataHelper<JSUint8ClampedArray, Uint8ClampedArray>(ctx, obj, numElements, data);
}

bool JSObjectGetInt16ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, int16_t* & data)
{
    return GetArrayBufferViewDataHelper<JSInt16Array, Int16Array>(ctx, obj, numElements, data);
}

bool JSObjectGetUint16ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, uint16_t* & data)
{
    return GetArrayBufferViewDataHelper<JSUint16Array, Uint16Array>(ctx, obj, numElements, data);
}

bool JSObjectGetInt32ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, int32_t* & data)
{
    return GetArrayBufferViewDataHelper<JSInt32Array, Int32Array>(ctx, obj, numElements, data);
}

bool JSObjectGetUint32ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, uint32_t* & data)
{
    return GetArrayBufferViewDataHelper<JSUint32Array, Uint32Array>(ctx, obj, numElements, data);
}

bool JSObjectGetFloat32ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, float* & data)
{
    return GetArrayBufferViewDataHelper<JSFloat32Array, Float32Array>(ctx, obj, numElements, data);
}

bool JSObjectGetFloat64ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, double* & data)
{
    return GetArrayBufferViewDataHelper<JSFloat64Array, Float64Array>(ctx, obj, numElements, data);
}
