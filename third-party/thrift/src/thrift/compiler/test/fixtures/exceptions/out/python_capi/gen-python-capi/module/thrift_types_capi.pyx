
#
# Autogenerated by Thrift
#
# DO NOT EDIT
#  @generated
#

from cpython.ref cimport PyObject
from libcpp.utility cimport move as __move
from folly.iobuf cimport (
    from_unique_ptr as __IOBuf_from_unique_ptr,
    IOBuf as __IOBuf,
)
from thrift.python.serializer import (
    deserialize as __deserialize,
    Protocol as __Protocol,
    serialize_iobuf as __serialize_iobuf,
)
import module.thrift_types as __thrift_types

cdef api int can_extract__module__Fiery(object __obj) except -1:
    return 1 if isinstance(__obj, __thrift_types.Fiery) else 0


cdef api object init__module__Fiery(object data):
    return __thrift_types.Fiery._fbthrift_from_internal_data(data)

cdef api int can_extract__module__Serious(object __obj) except -1:
    return 1 if isinstance(__obj, __thrift_types.Serious) else 0


cdef api object init__module__Serious(object data):
    return __thrift_types.Serious._fbthrift_from_internal_data(data)

cdef api int can_extract__module__ComplexFieldNames(object __obj) except -1:
    return 1 if isinstance(__obj, __thrift_types.ComplexFieldNames) else 0


cdef api object init__module__ComplexFieldNames(object data):
    return __thrift_types.ComplexFieldNames._fbthrift_from_internal_data(data)

cdef api int can_extract__module__CustomFieldNames(object __obj) except -1:
    return 1 if isinstance(__obj, __thrift_types.CustomFieldNames) else 0


cdef api object init__module__CustomFieldNames(object data):
    return __thrift_types.CustomFieldNames._fbthrift_from_internal_data(data)

cdef api int can_extract__module__ExceptionWithPrimitiveField(object __obj) except -1:
    return 1 if isinstance(__obj, __thrift_types.ExceptionWithPrimitiveField) else 0


cdef api object init__module__ExceptionWithPrimitiveField(object data):
    return __thrift_types.ExceptionWithPrimitiveField._fbthrift_from_internal_data(data)

cdef api int can_extract__module__ExceptionWithStructuredAnnotation(object __obj) except -1:
    return 1 if isinstance(__obj, __thrift_types.ExceptionWithStructuredAnnotation) else 0


cdef api object init__module__ExceptionWithStructuredAnnotation(object data):
    return __thrift_types.ExceptionWithStructuredAnnotation._fbthrift_from_internal_data(data)

cdef api int can_extract__module__Banal(object __obj) except -1:
    return 1 if isinstance(__obj, __thrift_types.Banal) else 0


cdef api object init__module__Banal(object data):
    return __thrift_types.Banal._fbthrift_from_internal_data(data)

