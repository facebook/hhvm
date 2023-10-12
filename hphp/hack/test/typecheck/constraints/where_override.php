<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Top { }
class Base extends Top { }
class Derived extends Base {
  public function bar(): void { echo 'bar'; }
}

class MyList<T> {
  public function foo(T $x):void where T as Base { }
  public function boo(T $x):void where T as Derived { }
}

// This is ok: we've weakened the constraints
class List1<T> extends MyList<T> {
  public function foo(T $x):void {
  }
  public function boo(T $x):void {
  }
}

// This is also ok: we've simply matched the constraints
class List2<T> extends MyList<T> {
  public function foo(T $x):void where T as Base {
  }
  public function boo(T $x):void where T as Derived {
    $x->bar();
  }
}

// This is also ok: we've weakened the constraints again
class List3<T> extends MyList<T> {
  public function foo(T $x):void where T as Top {
  }
  public function boo(T $x):void where T as Base {
  }
}
