#
# Autogenerated by Thrift for thrift/compiler/test/fixtures/includes/src/transitive.thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#


import folly.iobuf as _fbthrift_iobuf

from thrift.py3.reflection import (
    NumberType as __NumberType,
    StructType as __StructType,
    Qualifier as __Qualifier,
    StructSpec as __StructSpec,
    ListSpec as __ListSpec,
    SetSpec as __SetSpec,
    MapSpec as __MapSpec,
    FieldSpec as __FieldSpec,
)


import transitive.types as _transitive_types



def get_reflection__Foo() -> __StructSpec:
    spec: __StructSpec = __StructSpec._fbthrift_create(
        name="Foo",
        kind=__StructType.STRUCT,
        annotations={
        },
    )
    defaults = _transitive_types.Foo()
    spec.add_field(
        __FieldSpec._fbthrift_create(
            id=1,
            name="a",
            py_name="a",
            type=int,
            kind=__NumberType.I64,
            qualifier=__Qualifier.UNQUALIFIED,
            default=defaults.a,
            annotations={
            },
        ),
    )
    return spec
