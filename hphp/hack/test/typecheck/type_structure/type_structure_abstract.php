<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface IBase {}
interface IChild extends IBase {}

abstract class C {
  abstract const type T as IBase;
  public static function foo(): void {
    hh_show(type_structure(static::class, 'T')['classname']);
  }
}

class D extends C {
  const type T = IChild;
  public static function foo(): void {
    hh_show(type_structure(static::class, 'T')['classname']);
  }
}
