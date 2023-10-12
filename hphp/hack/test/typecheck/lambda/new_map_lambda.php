<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

enum MyEnum: string as string {
  GENDER = 'gender';
  INTEREST = 'interest';
}

class MyMap<Tk, Tv> {
  public function mapWithKey<Tu>(
    (function(Tk, Tv): Tu) $callback,
  ): MyMap<Tk, Tu> {
    throw new Exception();
  }
}
class C {

  private static function getLabel(MyEnum $option): string {
    return "a";
  }

  public function bong(MyMap<string, string> $m): void {
    // Different error position behaviour
    $x = $m->mapWithKey(($value, $name) ==> self::getLabel($value));
    $y = $m->mapWithKey((MyEnum $value, $name) ==> self::getLabel($value));
  }
}
