#
# Autogenerated by Thrift
#
# DO NOT EDIT
#  @generated
#

from __future__ import annotations

import folly.iobuf as _fbthrift_iobuf

from abc import ABCMeta as _fbthrift_ABCMeta
import module.thrift_abstract_types as _fbthrift_abstract_types
import thrift.python.types as _fbthrift_python_types
import thrift.python.exceptions as _fbthrift_python_exceptions



class Fiery(metaclass=_fbthrift_python_exceptions.GeneratedErrorMeta):
    _fbthrift_SPEC = (
        _fbthrift_python_types.FieldInfo(
            1,  # id
            _fbthrift_python_types.FieldQualifier.Unqualified, # qualifier
            "message",  # name
            "message",  # python name (from @python.Name annotation)
            _fbthrift_python_types.typeinfo_string,  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            8, # IDL type (see BaseTypeEnum)
        ),
    )

    _fbthrift_abstract_base_class = _fbthrift_abstract_types.Fiery


    @staticmethod
    def __get_thrift_name__() -> str:
        return "module.Fiery"

    @staticmethod
    def __get_thrift_uri__():
        return None

    @classmethod
    def _fbthrift_auto_migrate_enabled(cls):
        return False

    @staticmethod
    def __get_metadata__():
        return _fbthrift_metadata__exception_Fiery()


    def __str__(self):
        field = self.message
        if field is None:
            return str(field)
        return field

    def _to_python(self):
        return self

    def _to_mutable_python(self):
        from thrift.python import mutable_converter
        import importlib
        mutable_types = importlib.import_module("module.thrift_mutable_types")
        return mutable_converter.to_mutable_python_struct_or_union(mutable_types.Fiery, self)

    def _to_py3(self):
        import importlib
        py3_types = importlib.import_module("module.types")
        from thrift.py3 import converter
        return converter.to_py3_struct(py3_types.Fiery, self)

    def _to_py_deprecated(self):
        import importlib
        from thrift.util import converter
        try:
            py_deprecated_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_deprecated_types.Fiery, self)
        except ModuleNotFoundError:
            py_asyncio_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_asyncio_types.Fiery, self)
_fbthrift_Fiery = Fiery

class Serious(metaclass=_fbthrift_python_exceptions.GeneratedErrorMeta):
    _fbthrift_SPEC = (
        _fbthrift_python_types.FieldInfo(
            1,  # id
            _fbthrift_python_types.FieldQualifier.Optional, # qualifier
            "sonnet",  # name
            "not_sonnet",  # python name (from @python.Name annotation)
            _fbthrift_python_types.typeinfo_string,  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            8, # IDL type (see BaseTypeEnum)
        ),
    )

    _fbthrift_abstract_base_class = _fbthrift_abstract_types.Serious


    @staticmethod
    def __get_thrift_name__() -> str:
        return "module.Serious"

    @staticmethod
    def __get_thrift_uri__():
        return None

    @classmethod
    def _fbthrift_auto_migrate_enabled(cls):
        return False

    @staticmethod
    def __get_metadata__():
        return _fbthrift_metadata__exception_Serious()


    def __str__(self):
        field = self.not_sonnet
        if field is None:
            return str(field)
        return field

    def _to_python(self):
        return self

    def _to_mutable_python(self):
        from thrift.python import mutable_converter
        import importlib
        mutable_types = importlib.import_module("module.thrift_mutable_types")
        return mutable_converter.to_mutable_python_struct_or_union(mutable_types.Serious, self)

    def _to_py3(self):
        import importlib
        py3_types = importlib.import_module("module.types")
        from thrift.py3 import converter
        return converter.to_py3_struct(py3_types.Serious, self)

    def _to_py_deprecated(self):
        import importlib
        from thrift.util import converter
        try:
            py_deprecated_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_deprecated_types.Serious, self)
        except ModuleNotFoundError:
            py_asyncio_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_asyncio_types.Serious, self)
_fbthrift_Serious = Serious

class ComplexFieldNames(metaclass=_fbthrift_python_exceptions.GeneratedErrorMeta):
    _fbthrift_SPEC = (
        _fbthrift_python_types.FieldInfo(
            1,  # id
            _fbthrift_python_types.FieldQualifier.Unqualified, # qualifier
            "error_message",  # name
            "error_message",  # python name (from @python.Name annotation)
            _fbthrift_python_types.typeinfo_string,  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            8, # IDL type (see BaseTypeEnum)
        ),
        _fbthrift_python_types.FieldInfo(
            2,  # id
            _fbthrift_python_types.FieldQualifier.Unqualified, # qualifier
            "internal_error_message",  # name
            "internal_error_message",  # python name (from @python.Name annotation)
            _fbthrift_python_types.typeinfo_string,  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            8, # IDL type (see BaseTypeEnum)
        ),
    )

    _fbthrift_abstract_base_class = _fbthrift_abstract_types.ComplexFieldNames


    @staticmethod
    def __get_thrift_name__() -> str:
        return "module.ComplexFieldNames"

    @staticmethod
    def __get_thrift_uri__():
        return None

    @classmethod
    def _fbthrift_auto_migrate_enabled(cls):
        return False

    @staticmethod
    def __get_metadata__():
        return _fbthrift_metadata__exception_ComplexFieldNames()


    def __str__(self):
        field = self.internal_error_message
        if field is None:
            return str(field)
        return field

    def _to_python(self):
        return self

    def _to_mutable_python(self):
        from thrift.python import mutable_converter
        import importlib
        mutable_types = importlib.import_module("module.thrift_mutable_types")
        return mutable_converter.to_mutable_python_struct_or_union(mutable_types.ComplexFieldNames, self)

    def _to_py3(self):
        import importlib
        py3_types = importlib.import_module("module.types")
        from thrift.py3 import converter
        return converter.to_py3_struct(py3_types.ComplexFieldNames, self)

    def _to_py_deprecated(self):
        import importlib
        from thrift.util import converter
        try:
            py_deprecated_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_deprecated_types.ComplexFieldNames, self)
        except ModuleNotFoundError:
            py_asyncio_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_asyncio_types.ComplexFieldNames, self)
_fbthrift_ComplexFieldNames = ComplexFieldNames

class CustomFieldNames(metaclass=_fbthrift_python_exceptions.GeneratedErrorMeta):
    _fbthrift_SPEC = (
        _fbthrift_python_types.FieldInfo(
            1,  # id
            _fbthrift_python_types.FieldQualifier.Unqualified, # qualifier
            "error_message",  # name
            "error_message",  # python name (from @python.Name annotation)
            _fbthrift_python_types.typeinfo_string,  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            8, # IDL type (see BaseTypeEnum)
        ),
        _fbthrift_python_types.FieldInfo(
            2,  # id
            _fbthrift_python_types.FieldQualifier.Unqualified, # qualifier
            "internal_error_message",  # name
            "internal_error_message",  # python name (from @python.Name annotation)
            _fbthrift_python_types.typeinfo_string,  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            8, # IDL type (see BaseTypeEnum)
        ),
    )

    _fbthrift_abstract_base_class = _fbthrift_abstract_types.CustomFieldNames


    @staticmethod
    def __get_thrift_name__() -> str:
        return "module.CustomFieldNames"

    @staticmethod
    def __get_thrift_uri__():
        return None

    @classmethod
    def _fbthrift_auto_migrate_enabled(cls):
        return False

    @staticmethod
    def __get_metadata__():
        return _fbthrift_metadata__exception_CustomFieldNames()


    def __str__(self):
        field = self.internal_error_message
        if field is None:
            return str(field)
        return field

    def _to_python(self):
        return self

    def _to_mutable_python(self):
        from thrift.python import mutable_converter
        import importlib
        mutable_types = importlib.import_module("module.thrift_mutable_types")
        return mutable_converter.to_mutable_python_struct_or_union(mutable_types.CustomFieldNames, self)

    def _to_py3(self):
        import importlib
        py3_types = importlib.import_module("module.types")
        from thrift.py3 import converter
        return converter.to_py3_struct(py3_types.CustomFieldNames, self)

    def _to_py_deprecated(self):
        import importlib
        from thrift.util import converter
        try:
            py_deprecated_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_deprecated_types.CustomFieldNames, self)
        except ModuleNotFoundError:
            py_asyncio_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_asyncio_types.CustomFieldNames, self)
_fbthrift_CustomFieldNames = CustomFieldNames

class ExceptionWithPrimitiveField(metaclass=_fbthrift_python_exceptions.GeneratedErrorMeta):
    _fbthrift_SPEC = (
        _fbthrift_python_types.FieldInfo(
            1,  # id
            _fbthrift_python_types.FieldQualifier.Unqualified, # qualifier
            "message",  # name
            "message",  # python name (from @python.Name annotation)
            _fbthrift_python_types.typeinfo_string,  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            8, # IDL type (see BaseTypeEnum)
        ),
        _fbthrift_python_types.FieldInfo(
            2,  # id
            _fbthrift_python_types.FieldQualifier.Unqualified, # qualifier
            "error_code",  # name
            "error_code",  # python name (from @python.Name annotation)
            _fbthrift_python_types.typeinfo_i32,  # typeinfo
            None,  # default value
            None,  # adapter info
            True, # field type is primitive
            4, # IDL type (see BaseTypeEnum)
        ),
    )

    _fbthrift_abstract_base_class = _fbthrift_abstract_types.ExceptionWithPrimitiveField


    @staticmethod
    def __get_thrift_name__() -> str:
        return "module.ExceptionWithPrimitiveField"

    @staticmethod
    def __get_thrift_uri__():
        return None

    @classmethod
    def _fbthrift_auto_migrate_enabled(cls):
        return False

    @staticmethod
    def __get_metadata__():
        return _fbthrift_metadata__exception_ExceptionWithPrimitiveField()


    def __str__(self):
        field = self.message
        if field is None:
            return str(field)
        return field

    def _to_python(self):
        return self

    def _to_mutable_python(self):
        from thrift.python import mutable_converter
        import importlib
        mutable_types = importlib.import_module("module.thrift_mutable_types")
        return mutable_converter.to_mutable_python_struct_or_union(mutable_types.ExceptionWithPrimitiveField, self)

    def _to_py3(self):
        import importlib
        py3_types = importlib.import_module("module.types")
        from thrift.py3 import converter
        return converter.to_py3_struct(py3_types.ExceptionWithPrimitiveField, self)

    def _to_py_deprecated(self):
        import importlib
        from thrift.util import converter
        try:
            py_deprecated_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_deprecated_types.ExceptionWithPrimitiveField, self)
        except ModuleNotFoundError:
            py_asyncio_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_asyncio_types.ExceptionWithPrimitiveField, self)
_fbthrift_ExceptionWithPrimitiveField = ExceptionWithPrimitiveField

class ExceptionWithStructuredAnnotation(metaclass=_fbthrift_python_exceptions.GeneratedErrorMeta):
    _fbthrift_SPEC = (
        _fbthrift_python_types.FieldInfo(
            1,  # id
            _fbthrift_python_types.FieldQualifier.Unqualified, # qualifier
            "message_field",  # name
            "message_field",  # python name (from @python.Name annotation)
            _fbthrift_python_types.typeinfo_string,  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            8, # IDL type (see BaseTypeEnum)
        ),
        _fbthrift_python_types.FieldInfo(
            2,  # id
            _fbthrift_python_types.FieldQualifier.Unqualified, # qualifier
            "error_code",  # name
            "error_code",  # python name (from @python.Name annotation)
            _fbthrift_python_types.typeinfo_i32,  # typeinfo
            None,  # default value
            None,  # adapter info
            True, # field type is primitive
            4, # IDL type (see BaseTypeEnum)
        ),
    )

    _fbthrift_abstract_base_class = _fbthrift_abstract_types.ExceptionWithStructuredAnnotation


    @staticmethod
    def __get_thrift_name__() -> str:
        return "module.ExceptionWithStructuredAnnotation"

    @staticmethod
    def __get_thrift_uri__():
        return None

    @classmethod
    def _fbthrift_auto_migrate_enabled(cls):
        return False

    @staticmethod
    def __get_metadata__():
        return _fbthrift_metadata__exception_ExceptionWithStructuredAnnotation()


    def __str__(self):
        field = self.message_field
        if field is None:
            return str(field)
        return field

    def _to_python(self):
        return self

    def _to_mutable_python(self):
        from thrift.python import mutable_converter
        import importlib
        mutable_types = importlib.import_module("module.thrift_mutable_types")
        return mutable_converter.to_mutable_python_struct_or_union(mutable_types.ExceptionWithStructuredAnnotation, self)

    def _to_py3(self):
        import importlib
        py3_types = importlib.import_module("module.types")
        from thrift.py3 import converter
        return converter.to_py3_struct(py3_types.ExceptionWithStructuredAnnotation, self)

    def _to_py_deprecated(self):
        import importlib
        from thrift.util import converter
        try:
            py_deprecated_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_deprecated_types.ExceptionWithStructuredAnnotation, self)
        except ModuleNotFoundError:
            py_asyncio_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_asyncio_types.ExceptionWithStructuredAnnotation, self)
_fbthrift_ExceptionWithStructuredAnnotation = ExceptionWithStructuredAnnotation

class Banal(metaclass=_fbthrift_python_exceptions.GeneratedErrorMeta):
    _fbthrift_SPEC = (
    )

    _fbthrift_abstract_base_class = _fbthrift_abstract_types.Banal


    @staticmethod
    def __get_thrift_name__() -> str:
        return "module.Banal"

    @staticmethod
    def __get_thrift_uri__():
        return None

    @classmethod
    def _fbthrift_auto_migrate_enabled(cls):
        return False

    @staticmethod
    def __get_metadata__():
        return _fbthrift_metadata__exception_Banal()

    def _to_python(self):
        return self

    def _to_mutable_python(self):
        from thrift.python import mutable_converter
        import importlib
        mutable_types = importlib.import_module("module.thrift_mutable_types")
        return mutable_converter.to_mutable_python_struct_or_union(mutable_types.Banal, self)

    def _to_py3(self):
        import importlib
        py3_types = importlib.import_module("module.types")
        from thrift.py3 import converter
        return converter.to_py3_struct(py3_types.Banal, self)

    def _to_py_deprecated(self):
        import importlib
        from thrift.util import converter
        try:
            py_deprecated_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_deprecated_types.Banal, self)
        except ModuleNotFoundError:
            py_asyncio_types = importlib.import_module("module.ttypes")
            return converter.to_py_struct(py_asyncio_types.Banal, self)
_fbthrift_Banal = Banal

# This unfortunately has to be down here to prevent circular imports
import module.thrift_metadata as _fbthrift__module__thrift_metadata

_fbthrift_all_enums = [
]


def _fbthrift_metadata__exception_Fiery():
    return _fbthrift__module__thrift_metadata.gen_metadata_exception_Fiery()


def _fbthrift_metadata__exception_Serious():
    return _fbthrift__module__thrift_metadata.gen_metadata_exception_Serious()


def _fbthrift_metadata__exception_ComplexFieldNames():
    return _fbthrift__module__thrift_metadata.gen_metadata_exception_ComplexFieldNames()


def _fbthrift_metadata__exception_CustomFieldNames():
    return _fbthrift__module__thrift_metadata.gen_metadata_exception_CustomFieldNames()


def _fbthrift_metadata__exception_ExceptionWithPrimitiveField():
    return _fbthrift__module__thrift_metadata.gen_metadata_exception_ExceptionWithPrimitiveField()


def _fbthrift_metadata__exception_ExceptionWithStructuredAnnotation():
    return _fbthrift__module__thrift_metadata.gen_metadata_exception_ExceptionWithStructuredAnnotation()


def _fbthrift_metadata__exception_Banal():
    return _fbthrift__module__thrift_metadata.gen_metadata_exception_Banal()


_fbthrift_all_structs = [
    Fiery,
    Serious,
    ComplexFieldNames,
    CustomFieldNames,
    ExceptionWithPrimitiveField,
    ExceptionWithStructuredAnnotation,
    Banal,
]
_fbthrift_python_types.fill_specs(*_fbthrift_all_structs)



class _fbthrift_Raiser_doBland_args(metaclass=_fbthrift_python_types.StructMeta):
    _fbthrift_SPEC = (
    )


class _fbthrift_Raiser_doBland_result(metaclass=_fbthrift_python_types.StructMeta):
    _fbthrift_SPEC = (
    )


class _fbthrift_Raiser_doRaise_args(metaclass=_fbthrift_python_types.StructMeta):
    _fbthrift_SPEC = (
    )


class _fbthrift_Raiser_doRaise_result(metaclass=_fbthrift_python_types.StructMeta):
    _fbthrift_SPEC = (
        _fbthrift_python_types.FieldInfo(
            1,  # id
            _fbthrift_python_types.FieldQualifier.Optional, # qualifier
            "_ex0__b",  # name
            "_ex0__b",  # python name (from @python.Name annotation)
            lambda: _fbthrift_python_types.StructTypeInfo(Banal),  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            11, # IDL type (see BaseTypeEnum)
        ),
        _fbthrift_python_types.FieldInfo(
            2,  # id
            _fbthrift_python_types.FieldQualifier.Optional, # qualifier
            "_ex1__f",  # name
            "_ex1__f",  # python name (from @python.Name annotation)
            lambda: _fbthrift_python_types.StructTypeInfo(Fiery),  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            11, # IDL type (see BaseTypeEnum)
        ),
        _fbthrift_python_types.FieldInfo(
            3,  # id
            _fbthrift_python_types.FieldQualifier.Optional, # qualifier
            "_ex2__s",  # name
            "_ex2__s",  # python name (from @python.Name annotation)
            lambda: _fbthrift_python_types.StructTypeInfo(Serious),  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            11, # IDL type (see BaseTypeEnum)
        ),
    )


class _fbthrift_Raiser_get200_args(metaclass=_fbthrift_python_types.StructMeta):
    _fbthrift_SPEC = (
    )


class _fbthrift_Raiser_get200_result(metaclass=_fbthrift_python_types.StructMeta):
    _fbthrift_SPEC = (
        _fbthrift_python_types.FieldInfo(
            0,  # id
            _fbthrift_python_types.FieldQualifier.Optional, # qualifier
            "success",  # name
            "success", # name
            _fbthrift_python_types.typeinfo_string,  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
        ),
    )


class _fbthrift_Raiser_get500_args(metaclass=_fbthrift_python_types.StructMeta):
    _fbthrift_SPEC = (
    )


class _fbthrift_Raiser_get500_result(metaclass=_fbthrift_python_types.StructMeta):
    _fbthrift_SPEC = (
        _fbthrift_python_types.FieldInfo(
            0,  # id
            _fbthrift_python_types.FieldQualifier.Optional, # qualifier
            "success",  # name
            "success", # name
            _fbthrift_python_types.typeinfo_string,  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
        ),
        _fbthrift_python_types.FieldInfo(
            1,  # id
            _fbthrift_python_types.FieldQualifier.Optional, # qualifier
            "_ex0__f",  # name
            "_ex0__f",  # python name (from @python.Name annotation)
            lambda: _fbthrift_python_types.StructTypeInfo(Fiery),  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            11, # IDL type (see BaseTypeEnum)
        ),
        _fbthrift_python_types.FieldInfo(
            2,  # id
            _fbthrift_python_types.FieldQualifier.Optional, # qualifier
            "_ex1__b",  # name
            "_ex1__b",  # python name (from @python.Name annotation)
            lambda: _fbthrift_python_types.StructTypeInfo(Banal),  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            11, # IDL type (see BaseTypeEnum)
        ),
        _fbthrift_python_types.FieldInfo(
            3,  # id
            _fbthrift_python_types.FieldQualifier.Optional, # qualifier
            "_ex2__s",  # name
            "_ex2__s",  # python name (from @python.Name annotation)
            lambda: _fbthrift_python_types.StructTypeInfo(Serious),  # typeinfo
            None,  # default value
            None,  # adapter info
            False, # field type is primitive
            11, # IDL type (see BaseTypeEnum)
        ),
    )



_fbthrift_python_types.fill_specs(
    _fbthrift_Raiser_doBland_args,
    _fbthrift_Raiser_doBland_result,
    _fbthrift_Raiser_doRaise_args,
    _fbthrift_Raiser_doRaise_result,
    _fbthrift_Raiser_get200_args,
    _fbthrift_Raiser_get200_result,
    _fbthrift_Raiser_get500_args,
    _fbthrift_Raiser_get500_result,
)
