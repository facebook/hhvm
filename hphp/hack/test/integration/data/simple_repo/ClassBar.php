<?hh // strict
<<file: __EnableUnstableFeatures('modules')>>

module bar;

internal class CI {
  internal int $x = 0;
  static internal int $y = 0;
}

abstract class X {
  abstract internal function foo(): CI;
}

class Y {
  internal function foo(): CI {
    return new CI();
  }

  public function foobar(): void {
  }

  static internal function barfoo(): void {
  }
}
