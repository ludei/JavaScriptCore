//
//  TypedArrays.h
//  JSTypedArrays.h
//
//  Created by Imanol Fernandez Gorostizaga on 26/04/13.
//  Copyright (c) 2013 Ideateca. All rights reserved.
//

#ifndef JavaScriptCore_TypedArrays_h
#define JavaScriptCore_TypedArrays_h

#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSValueRef.h>

typedef enum {
    kJSArrayBufferViewInt8,
    kJSArrayBufferViewUint8,
    kJSArrayBufferViewUint8Clamped,
    kJSArrayBufferViewInt16,
    kJSArrayBufferViewUint16,
    kJSArrayBufferViewInt32,
    kJSArrayBufferViewUint32,
    kJSArrayBufferViewFloat32,
    kJSArrayBufferViewFloat64,
    kJSArrayBufferViewDataView
} JSObjectArrayBufferViewType;


//Constructors

/*
 * Makes an ArrayBufferView object with the desided type and element count
 */
JSObjectRef JSObjectMakeArrayBufferView(JSContextRef ctx, JSObjectArrayBufferViewType viewType, unsigned int numElements);

/*
 * Makes an ArrayBuffer object with the desided bytes count
 */
JSObjectRef JSObjectMakeArrayBuffer(JSContextRef ctx, unsigned int nBytes);


//Type check

/*
 * Tests if the object supports get raw data methods (it can be an ArrayBufferView or an ArrayBuffer);
 */
bool JSObjectIsTypedArray(JSContextRef ctx, JSObjectRef obj);

/*
 * Tests if the object is an ArrayBufferView object
 */
bool JSObjectIsArrayBufferView(JSContextRef ctx, JSObjectRef obj);

/*
 * Tests if the object is an ArrayBufferView object of the desired type
 */
bool JSObjectIsArrayBufferViewOfType(JSContextRef ctx, JSObjectRef obj, JSObjectArrayBufferViewType viewType);

/*
 * Get the type of elements in a typed array, or TYPE_DATAVIEW if a DataView
 */
bool JSObjectGetArrayBufferViewType(JSContextRef ctx, JSObjectRef obj, JSObjectArrayBufferViewType & viewType);

/*
 * Tests if the object is an ArrayBuffer object
 */
bool JSObjectIsArrayBuffer(JSContextRef ctx, JSObjectRef obj);

/*
 * Tests if the object is a normal Array
 */
bool JSObjectIsUntypedArray(JSContextRef ctx, JSObjectRef obj);

/*
 * Gets the length of an untyped array
 */
unsigned int JSObjectGetUntypedArrayLength(JSContextRef ctx, JSObjectRef obj);

//Get Data

/*
 * Gets the raw data of any typed array object (ArrayBuffer or ArrayBufferView)
 */
bool JSObjectGetTypedArrayData(JSContextRef ctx, JSObjectRef obj, unsigned int & numBytes, uint8_t * & data);

//Explicit Getters
bool JSObjectGetInt8ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, int8_t* & data);

bool JSObjectGetUint8ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, uint8_t* & data);

bool JSObjectGetUint8ClampedArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, uint8_t* & data);

bool JSObjectGetInt16ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, int16_t* & data);

bool JSObjectGetUint16ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, uint16_t* & data);

bool JSObjectGetInt32ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, int32_t* & data);

bool JSObjectGetUint32ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, uint32_t* & data);

bool JSObjectGetFloat32ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, float* & data);

bool JSObjectGetFloat64ArrayViewData(JSContextRef ctx, JSObjectRef obj, unsigned int & numElements, double* & data);

#endif
