<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Action<-T> {
  public function doIt(T $x): void {}
}

class CAction<T as C> extends Action<T> {
  public function doIt(T $x): void {
    $x->bar();
  }
}

class B {
  public function foo<T as B>(): Action<T> {
    return new Action();
  }
}
class C extends B {
  public function foo<T as C>(): Action<T> {
    return new CAction();
  }
  public function bar(): void {
    echo 'bar';
  }
}

function CallOnB(B $b): void {
  $act = $b->foo();
  $act->doIt(new B());
}

function TestIt(): void {
  $c = new C();
  CallOnB($c);
}

function main(): void {
  TestIt();
}
