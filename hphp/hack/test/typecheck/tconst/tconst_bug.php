<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expectVecString(vec<string> $vs):void {
}
class W {
  public function testit(vec<StringType> $x, StringType $st): void {
    $exactst = new StringType();
    hh_show(StringType::foo<>);
    hh_show(StringType::bar<>);
    hh_show(($boxes) ==> $exactst->inst_foo($boxes));
    hh_show(($boxes) ==> $st->inst_foo($boxes));
    hh_show(($boxes) ==> $exactst->inst_bar($boxes));
    hh_show(($boxes) ==> $st->inst_bar($boxes));
    expectVecString(StringType::foo($x));
    expectVecString(StringType::bar($x));
  }
}

class StringType extends ExposeAsType {
  const type TExposeAs = string;
}

abstract class ExposeAsType {
  abstract const type TExposeAs;
  /** Forces allowing subclasses of this in place of this */
  const type TThis = this;

  public static final function foo(
    vec<this::TThis> $boxes,
  ): vec<this::TThis::TExposeAs> {
    return vec[];
  }

  // This fails, but why?
  public static final function bar(
    vec<this> $boxes,
  ): vec<this::TExposeAs> {
    return vec[];
  }

  public function inst_foo(
    vec<this::TThis> $boxes,
  ): vec<this::TThis::TExposeAs> {
    return vec[];
  }
  public function inst_bar(
    vec<this> $boxes,
  ): vec<this::TExposeAs> {
    return vec[];
  }
}
