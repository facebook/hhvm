<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface IBase {}
interface IChild extends IBase {}

abstract class C {
  abstract const type T as IBase;
  public static function foo(): void {
    type_structure(self::class, 'T');
  }
}

class D extends C {
  const type T = IChild;
  public static function foo(): void {
    hh_show(type_structure(self::class, 'T')['classname']);
  }
}
