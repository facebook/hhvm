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

abstract class WithAbstractConst {
  abstract const type ABS_WO_CONSTRAINT;
  abstract const type ABS_WITH_CONSTRAINT as string;
  const type NESTED = WithConst;

  abstract public function with_abstract_type_constants(this::ABS_WO_CONSTRAINT $arg)
  : this::ABS_WITH_CONSTRAINT;
}

class WithConst {
  const float CFLOAT = 1.2;
  const SomeEnum CENUM = SomeEnum::SECOND;
  const type WITH_CONSTRAINT = A0;
  const type WITH_THIS = this::WITH_CONSTRAINT;
  const type TYPECONST as num = int;

  public function with_type_constants(this::TYPECONST $arg1,
                                      this::WITH_CONSTRAINT $arg2): void {}
}

const shape('x' => int, 'y' => SecondEnum) SHAPE =
  shape('x' => 5, 'y' => SecondEnum::SECOND);
const (int, ?(string, float)) OPTION = tuple(7, null);
const array<string, int> ARR = array('a' => 1, 'b' => 2);
const darray<string, int> AGE_RANGE = darray['min' => 21];
const varray<string> MAP_INDEX = varray['MAP_1', 'MAP_2'];
const classname<WithConst> CLASSNAME = WithConst::class;
const keyset<string> KEYSET = keyset['a', 'b'];
const TypedefForString TYPEDEF = "hello";

function with_constants(): void {
  $a = WithConst::CFLOAT;
  $b = WithConst::CENUM;
  $c = SHAPE;
  $d = OPTION;
  $e = ARR;
  $f = AGE_RANGE;
  $g = MAP_INDEX;
  $h = CLASSNAME;
  $i = KEYSET;
  $j = TYPEDEF;
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
