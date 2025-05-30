#
# Autogenerated by Thrift for thrift/compiler/test/fixtures/enums/src/module.thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#
cimport cython as __cython
from cython.operator cimport dereference as deref
from libcpp.utility cimport move as cmove
from thrift.py3.types cimport (
    assign_unique_ptr,
    assign_shared_ptr,
    assign_shared_const_ptr,
    bytes_to_string,
    make_unique,
    make_shared,
    make_const_shared,
)
cimport thrift.py3.types
from thrift.py3.types cimport (
    reset_field as __reset_field,
    StructFieldsSetter as __StructFieldsSetter
)

from thrift.py3.types cimport const_pointer_cast
from thrift.python.types cimport BadEnum as _fbthrift_BadEnum
from thrift.py3.types import _from_python_or_raise
from thrift.py3.types cimport _ensure_py3_container_or_raise


import test.fixtures.enums.module.types as _test_fixtures_enums_module_types
from test.fixtures.enums.module.containers_FBTHRIFT_ONLY_DO_NOT_USE import (
    Set__i32,
)


@__cython.auto_pickle(False)
cdef class __SomeStruct_FieldsSetter(__StructFieldsSetter):

    @staticmethod
    cdef __SomeStruct_FieldsSetter _fbthrift_create(_test_fixtures_enums_module_cbindings.cSomeStruct* struct_cpp_obj):
        cdef __SomeStruct_FieldsSetter __fbthrift_inst = __SomeStruct_FieldsSetter.__new__(__SomeStruct_FieldsSetter)
        __fbthrift_inst._struct_cpp_obj = struct_cpp_obj
        __fbthrift_inst._setters[__cstring_view(<const char*>"reasonable")] = __SomeStruct_FieldsSetter._set_field_0
        __fbthrift_inst._setters[__cstring_view(<const char*>"fine")] = __SomeStruct_FieldsSetter._set_field_1
        __fbthrift_inst._setters[__cstring_view(<const char*>"questionable")] = __SomeStruct_FieldsSetter._set_field_2
        __fbthrift_inst._setters[__cstring_view(<const char*>"tags")] = __SomeStruct_FieldsSetter._set_field_3
        return __fbthrift_inst

    cdef void set_field(__SomeStruct_FieldsSetter self, const char* name, object value) except *:
        cdef __cstring_view cname = __cstring_view(name)
        cdef cumap[__cstring_view, __SomeStruct_FieldsSetterFunc].iterator found = self._setters.find(cname)
        if found == self._setters.end():
            raise TypeError(f"invalid field name {name.decode('utf-8')}")
        deref(found).second(self, value)

    cdef void _set_field_0(self, _fbthrift_value) except *:
        # for field reasonable
        if _fbthrift_value is None:
            __reset_field[_test_fixtures_enums_module_cbindings.cSomeStruct](deref(self._struct_cpp_obj), 0)
            return
        if not isinstance(_fbthrift_value, _fbthrift_BadEnum) and not isinstance(_fbthrift_value, _test_fixtures_enums_module_types.Metasyntactic):
            raise TypeError(f'field reasonable value: {repr(_fbthrift_value)} is not of the enum type { _test_fixtures_enums_module_types.Metasyntactic }.')
        deref(self._struct_cpp_obj).reasonable_ref().assign(<_test_fixtures_enums_module_cbindings.cMetasyntactic><int>_fbthrift_value)

    cdef void _set_field_1(self, _fbthrift_value) except *:
        # for field fine
        if _fbthrift_value is None:
            __reset_field[_test_fixtures_enums_module_cbindings.cSomeStruct](deref(self._struct_cpp_obj), 1)
            return
        if not isinstance(_fbthrift_value, _fbthrift_BadEnum) and not isinstance(_fbthrift_value, _test_fixtures_enums_module_types.Metasyntactic):
            raise TypeError(f'field fine value: {repr(_fbthrift_value)} is not of the enum type { _test_fixtures_enums_module_types.Metasyntactic }.')
        deref(self._struct_cpp_obj).fine_ref().assign(<_test_fixtures_enums_module_cbindings.cMetasyntactic><int>_fbthrift_value)

    cdef void _set_field_2(self, _fbthrift_value) except *:
        # for field questionable
        if _fbthrift_value is None:
            __reset_field[_test_fixtures_enums_module_cbindings.cSomeStruct](deref(self._struct_cpp_obj), 2)
            return
        if not isinstance(_fbthrift_value, _fbthrift_BadEnum) and not isinstance(_fbthrift_value, _test_fixtures_enums_module_types.Metasyntactic):
            raise TypeError(f'field questionable value: {repr(_fbthrift_value)} is not of the enum type { _test_fixtures_enums_module_types.Metasyntactic }.')
        deref(self._struct_cpp_obj).questionable_ref().assign(<_test_fixtures_enums_module_cbindings.cMetasyntactic><int>_fbthrift_value)

    cdef void _set_field_3(self, _fbthrift_value) except *:
        # for field tags
        if _fbthrift_value is None:
            __reset_field[_test_fixtures_enums_module_cbindings.cSomeStruct](deref(self._struct_cpp_obj), 3)
            return
        deref(self._struct_cpp_obj).tags_ref().assign(_test_fixtures_enums_module_types.Set__i32__make_instance(_fbthrift_value))


@__cython.auto_pickle(False)
cdef class __MyStruct_FieldsSetter(__StructFieldsSetter):

    @staticmethod
    cdef __MyStruct_FieldsSetter _fbthrift_create(_test_fixtures_enums_module_cbindings.cMyStruct* struct_cpp_obj):
        cdef __MyStruct_FieldsSetter __fbthrift_inst = __MyStruct_FieldsSetter.__new__(__MyStruct_FieldsSetter)
        __fbthrift_inst._struct_cpp_obj = struct_cpp_obj
        __fbthrift_inst._setters[__cstring_view(<const char*>"me2_3")] = __MyStruct_FieldsSetter._set_field_0
        __fbthrift_inst._setters[__cstring_view(<const char*>"me3_n3")] = __MyStruct_FieldsSetter._set_field_1
        __fbthrift_inst._setters[__cstring_view(<const char*>"me1_t1")] = __MyStruct_FieldsSetter._set_field_2
        __fbthrift_inst._setters[__cstring_view(<const char*>"me1_t2")] = __MyStruct_FieldsSetter._set_field_3
        return __fbthrift_inst

    cdef void set_field(__MyStruct_FieldsSetter self, const char* name, object value) except *:
        cdef __cstring_view cname = __cstring_view(name)
        cdef cumap[__cstring_view, __MyStruct_FieldsSetterFunc].iterator found = self._setters.find(cname)
        if found == self._setters.end():
            raise TypeError(f"invalid field name {name.decode('utf-8')}")
        deref(found).second(self, value)

    cdef void _set_field_0(self, _fbthrift_value) except *:
        # for field me2_3
        if _fbthrift_value is None:
            __reset_field[_test_fixtures_enums_module_cbindings.cMyStruct](deref(self._struct_cpp_obj), 0)
            return
        if not isinstance(_fbthrift_value, _fbthrift_BadEnum) and not isinstance(_fbthrift_value, _test_fixtures_enums_module_types.MyEnum2):
            raise TypeError(f'field me2_3 value: {repr(_fbthrift_value)} is not of the enum type { _test_fixtures_enums_module_types.MyEnum2 }.')
        deref(self._struct_cpp_obj).me2_3_ref().assign(<_test_fixtures_enums_module_cbindings.cMyEnum2><int>_fbthrift_value)

    cdef void _set_field_1(self, _fbthrift_value) except *:
        # for field me3_n3
        if _fbthrift_value is None:
            __reset_field[_test_fixtures_enums_module_cbindings.cMyStruct](deref(self._struct_cpp_obj), 1)
            return
        if not isinstance(_fbthrift_value, _fbthrift_BadEnum) and not isinstance(_fbthrift_value, _test_fixtures_enums_module_types.MyEnum3):
            raise TypeError(f'field me3_n3 value: {repr(_fbthrift_value)} is not of the enum type { _test_fixtures_enums_module_types.MyEnum3 }.')
        deref(self._struct_cpp_obj).me3_n3_ref().assign(<_test_fixtures_enums_module_cbindings.cMyEnum3><int>_fbthrift_value)

    cdef void _set_field_2(self, _fbthrift_value) except *:
        # for field me1_t1
        if _fbthrift_value is None:
            __reset_field[_test_fixtures_enums_module_cbindings.cMyStruct](deref(self._struct_cpp_obj), 2)
            return
        if not isinstance(_fbthrift_value, _fbthrift_BadEnum) and not isinstance(_fbthrift_value, _test_fixtures_enums_module_types.MyEnum1):
            raise TypeError(f'field me1_t1 value: {repr(_fbthrift_value)} is not of the enum type { _test_fixtures_enums_module_types.MyEnum1 }.')
        deref(self._struct_cpp_obj).me1_t1_ref().assign(<_test_fixtures_enums_module_cbindings.cMyEnum1><int>_fbthrift_value)

    cdef void _set_field_3(self, _fbthrift_value) except *:
        # for field me1_t2
        if _fbthrift_value is None:
            __reset_field[_test_fixtures_enums_module_cbindings.cMyStruct](deref(self._struct_cpp_obj), 3)
            return
        if not isinstance(_fbthrift_value, _fbthrift_BadEnum) and not isinstance(_fbthrift_value, _test_fixtures_enums_module_types.MyEnum1):
            raise TypeError(f'field me1_t2 value: {repr(_fbthrift_value)} is not of the enum type { _test_fixtures_enums_module_types.MyEnum1 }.')
        deref(self._struct_cpp_obj).me1_t2_ref().assign(<_test_fixtures_enums_module_cbindings.cMyEnum1><int>_fbthrift_value)

