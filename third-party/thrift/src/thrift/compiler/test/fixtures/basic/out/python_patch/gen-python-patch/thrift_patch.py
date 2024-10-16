# @generated

import typing
import typing as _typing

from common.thrift.patch.detail.dynamic_patch import (
    BaseStructPatch,
    MapPatch,
    OptionalFieldPatch,
)

from common.thrift.patch.detail.py_bindings.DynamicPatch import (
    StringPatch,
)

import test.fixtures.basic.module.thrift_types
from test.fixtures.basic.module.thrift_types import (
    MyStruct,
    Containers,
    MyDataItem,
    MyUnion,
    MyException,
    MyExceptionWithMessage,
    ReservedKeyword,
    UnionToBeRenamed,
)


class MyStructPatch(BaseStructPatch[MyStruct]):
    @property
    def MyIntField(self) -> OptionalFieldPatch[
            int,
            I64Patch]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            1)
    @property
    def MyStringField(self) -> OptionalFieldPatch[
            str,
            StringPatch]:

        return OptionalFieldPatch(
            lambda patch: patch.as_string_patch(),
            self._patch,
            2)
    @property
    def MyDataField(self) -> OptionalFieldPatch[
            test.fixtures.basic.module.thrift_types.MyDataItem,
            MyDataItemPatch]:

        return OptionalFieldPatch(
            lambda patch: MyDataItemPatch(patch),
            self._patch,
            3)
    @property
    def myEnum(self) -> OptionalFieldPatch[
            test.fixtures.basic.module.thrift_types.MyEnum,
            test.fixtures.basic.module.thrift_types.MyEnum]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            4)
    @property
    def oneway(self) -> OptionalFieldPatch[
            bool,
            BoolPatch]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            5)
    @property
    def readonly(self) -> OptionalFieldPatch[
            bool,
            BoolPatch]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            6)
    @property
    def idempotent(self) -> OptionalFieldPatch[
            bool,
            BoolPatch]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            7)
    @property
    def floatSet(self) -> OptionalFieldPatch[
            _typing.AbstractSet[float],
            SetPatch[float]]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            8)
    @property
    def no_hack_codegen_field(self) -> OptionalFieldPatch[
            str,
            StringPatch]:

        return OptionalFieldPatch(
            lambda patch: patch.as_string_patch(),
            self._patch,
            9)

class ContainersPatch(BaseStructPatch[Containers]):
    @property
    def I32List(self) -> OptionalFieldPatch[
            _typing.Sequence[int],
            ListPatch[int]]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            1)
    @property
    def StringSet(self) -> OptionalFieldPatch[
            _typing.AbstractSet[str],
            SetPatch[str]]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            2)
    @property
    def StringToI64Map(self) -> OptionalFieldPatch[
            _typing.Mapping[str, int],
            MapPatch[str, I64Patch]]:

        return OptionalFieldPatch(
            lambda patch: MapPatch(lambda patch: , patch.as_map_patch()),
            self._patch,
            3)

class MyDataItemPatch(BaseStructPatch[MyDataItem]):

class MyUnionPatch(BaseStructPatch[MyUnion]):
    @property
    def myEnum(self) -> OptionalFieldPatch[
            test.fixtures.basic.module.thrift_types.MyEnum,
            test.fixtures.basic.module.thrift_types.MyEnum]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            1)
    @property
    def myStruct(self) -> OptionalFieldPatch[
            test.fixtures.basic.module.thrift_types.MyStruct,
            MyStructPatch]:

        return OptionalFieldPatch(
            lambda patch: MyStructPatch(patch),
            self._patch,
            2)
    @property
    def myDataItem(self) -> OptionalFieldPatch[
            test.fixtures.basic.module.thrift_types.MyDataItem,
            MyDataItemPatch]:

        return OptionalFieldPatch(
            lambda patch: MyDataItemPatch(patch),
            self._patch,
            3)
    @property
    def floatSet(self) -> OptionalFieldPatch[
            _typing.AbstractSet[float],
            SetPatch[float]]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            4)

class MyExceptionPatch(BaseStructPatch[MyException]):
    @property
    def MyIntField(self) -> OptionalFieldPatch[
            int,
            I64Patch]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            1)
    @property
    def MyStringField(self) -> OptionalFieldPatch[
            str,
            StringPatch]:

        return OptionalFieldPatch(
            lambda patch: patch.as_string_patch(),
            self._patch,
            2)
    @property
    def myStruct(self) -> OptionalFieldPatch[
            test.fixtures.basic.module.thrift_types.MyStruct,
            MyStructPatch]:

        return OptionalFieldPatch(
            lambda patch: MyStructPatch(patch),
            self._patch,
            3)
    @property
    def myUnion(self) -> OptionalFieldPatch[
            test.fixtures.basic.module.thrift_types.MyUnion,
            MyUnionPatch]:

        return OptionalFieldPatch(
            lambda patch: MyUnionPatch(patch),
            self._patch,
            4)

class MyExceptionWithMessagePatch(BaseStructPatch[MyExceptionWithMessage]):
    @property
    def MyIntField(self) -> OptionalFieldPatch[
            int,
            I64Patch]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            1)
    @property
    def MyStringField(self) -> OptionalFieldPatch[
            str,
            StringPatch]:

        return OptionalFieldPatch(
            lambda patch: patch.as_string_patch(),
            self._patch,
            2)
    @property
    def myStruct(self) -> OptionalFieldPatch[
            test.fixtures.basic.module.thrift_types.MyStruct,
            MyStructPatch]:

        return OptionalFieldPatch(
            lambda patch: MyStructPatch(patch),
            self._patch,
            3)
    @property
    def myUnion(self) -> OptionalFieldPatch[
            test.fixtures.basic.module.thrift_types.MyUnion,
            MyUnionPatch]:

        return OptionalFieldPatch(
            lambda patch: MyUnionPatch(patch),
            self._patch,
            4)

class ReservedKeywordPatch(BaseStructPatch[ReservedKeyword]):
    @property
    def reserved_field(self) -> OptionalFieldPatch[
            int,
            I32Patch]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            1)

class UnionToBeRenamedPatch(BaseStructPatch[UnionToBeRenamed]):
    @property
    def reserved_field(self) -> OptionalFieldPatch[
            int,
            I32Patch]:

        return OptionalFieldPatch(
            lambda patch: ,
            self._patch,
            1)
