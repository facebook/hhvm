<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Base { }
class Derived extends Base {
  public function bar(): void { echo 'bar'; }
}

class MyList<T> {
  public function foo(T $x):void where T as Base { }
  public function boo(T $x):void where T as Derived { }
}

// This is bad: we've strengthened a constraint
class List4<T> extends MyList<T> {
  public function foo(T $x):void where T as Derived {
    $x->bar();
  }
  public function boo(T $x):void {
  }
}


function BreakIt(MyList<Base> $l):void {
  $l->foo(new Base());
}


function TestIt():void {
  $x = new List4();
  BreakIt($x);
}

function main(): void {
  TestIt();
}
