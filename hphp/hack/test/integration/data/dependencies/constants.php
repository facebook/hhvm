<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

type TypedefForString = string;

enum SomeEnum: int {
  FIRST = 2;
  SECOND = 3;
}

enum SecondEnum: string {
  FIRST = "4";
  SECOND = "5";
}

type SomeEnumType = SomeEnum;

enum ThirdEnum: SomeEnumType {
  MUMBLE = SomeEnum::FIRST;
}

function with_enum_type_alias(ThirdEnum $_): void {}

function with_enum_class_name(): void {
  $_ = SomeEnum::class;
}

abstract class WithAbstractConst {
  abstract const type ABS_WO_CONSTRAINT;
  abstract const type ABS_WITH_CONSTRAINT as string;
  const type NESTED = WithConst;

  abstract public function with_abstract_type_constants(this::ABS_WO_CONSTRAINT $arg)
  : this::ABS_WITH_CONSTRAINT;
}

class WithConst {
  const float CFLOAT = 1.2;
  const string CSTRING = 'foo';
  const SomeEnum CENUM = SomeEnum::SECOND;
  const CWITHOUT_HINT = 'constant with inferred type';
  const type WITH_CONSTRAINT = A0;
  const type WITH_THIS = this::WITH_CONSTRAINT;
  const type TYPECONST = int;

  public function with_type_constants(this::TYPECONST $arg1,
                                      this::WITH_CONSTRAINT $arg2): void {}
}

const shape('x' => int, 'y' => SecondEnum) SHAPE1 =
  shape('x' => 5, 'y' => SecondEnum::SECOND);
const shape(WithConst::CSTRING => int) SHAPE2 =
  shape(WithConst::CSTRING => 42);
const shape('a' => int, 'b' => string, ...) SHAPE3 =
  shape('a' => 42, 'b' => 'foo', 'c' => 3.14);
const (int, ?(string, float)) OPTION = tuple(7, null);
const darray<string, int> ARR = darray['a' => 1, 'b' => 2];
const darray<string, int> AGE_RANGE = darray['min' => 21];
const varray<string> MAP_INDEX = varray['MAP_1', 'MAP_2'];
const classname<WithConst> CLASSNAME = WithConst::class;
const keyset<string> KEYSET = keyset['a', 'b'];
const TypedefForString TYPEDEF = "hello";
const varray_or_darray<int> CVARRAY_OR_DARRAY = varray[1, 2, 3];

function with_constants(): void {
  $_ = WithConst::CFLOAT;
  $_ = WithConst::CENUM;
  $_ = SHAPE1;
  $_ = OPTION;
  $_ = ARR;
  $_ = AGE_RANGE;
  $_ = MAP_INDEX;
  $_ = CLASSNAME;
  $_ = KEYSET;
  $_ = TYPEDEF;
  $_ = SHAPE2;
  $_ = CVARRAY_OR_DARRAY;
  $_ = SHAPE3;
  $_ = WithConst::CWITHOUT_HINT;
}

function with_type_constants(WithAbstractConst::NESTED::WITH_THIS $arg)
: WithConst::TYPECONST {
  return 1;
}

class WithStaticProperty {
  public static Map<SomeEnum, string> $map =
    Map {SomeEnum::FIRST => 'first', SomeEnum::SECOND => 'second'};
  public static Vector<A> $vector = Vector {};
}

function with_static_property(): void {
  $a = WithStaticProperty::$map;
  $b = WithStaticProperty::$vector;
}

function with_switch(SomeEnum $x): void {
  switch ($x) {
    case SomeEnum::FIRST:
      return;
    default:
      return;
  }
}

type SomeShape = shape(WithConst::CSTRING => mixed);

function with_shape_type_alias(SomeShape $_): void {}
