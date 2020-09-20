<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function expectVecString(vec<string> $vs):void {
}
class W {
  public function testit(vec<StringType> $x, StringType $st): void {
    $exactst = new StringType();
    hh_show(class_meth(StringType::class, 'foo'));
    hh_show(class_meth(StringType::class, 'bar'));
    hh_show(inst_meth($exactst, 'inst_foo'));
    hh_show(inst_meth($st, 'inst_foo'));
    hh_show(inst_meth($exactst, 'inst_bar'));
    hh_show(inst_meth($st, 'inst_bar'));
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
