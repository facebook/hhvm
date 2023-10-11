<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface IHasFoo {
  public function foo(): void;
}

interface ICanCallFoo<-T> {
  public function callFoo(T $x): void;
}

interface IHasBar {
  public function bar(): void;
}

class COnlyBar implements IHasBar {
  public function bar(): void {
    echo 'bar';
  }
}

final class GenericClass<T as IHasFoo> implements ICanCallFoo<T> {
  public function callFoo(T $item): void {
    $item->foo();
  }
}

class TestClass {
  public static function createGenericClass(): ICanCallFoo<IHasBar> {
    return new GenericClass();
  }
  public static function BreakIt(): void {
    $a = self::createGenericClass();
    $a->callFoo(new COnlyBar());
  }
}

function main(): void {
  TestClass::BreakIt();
}
