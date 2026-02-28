# @generated
from __future__ import annotations

import typing
import typing as _typing

from common.thrift.patch.detail.dynamic_patch import (
    BaseStructPatch,
    BaseUnionPatch,
    ListPatch,
    SetPatch,
    MapPatch,
    OptionalFieldPatch,
    UnqualifiedFieldPatch,
)

from common.thrift.patch.detail.py_bindings.DynamicPatch import (
    BoolPatch,
    BytePatch,
    I16Patch,
    I32Patch,
    I64Patch,
    FloatPatch,
    DoublePatch,
    StringPatch,
    BinaryPatch,
    StructPatch as DynamicStructPatch,
    UnionPatch as DynamicUnionPatch,
    DynamicPatch,
    EnumPatch
)

import thrift.python.types as _fbthrift_python_types
import folly.iobuf as _fbthrift_iobuf

import apache.thrift.type.id.thrift_types as _fbthrift__apache__thrift__type__id__thrift_types
import apache.thrift.type.gen_safe_patch_id.thrift_types as _fbthrift_safe_patch_types


