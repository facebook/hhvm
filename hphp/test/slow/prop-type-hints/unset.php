<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function err($errno, $errmsg, $errfile, $errline) :mixed{
  echo "$errmsg in $errfile on line $errline\n";
  throw new Exception();
}

type FakeType1<T> = string;
type FakeType2<T> = mixed;
type FakeType3<T> = T;

class C {
  public $x;
}

class A {
  public int $x;
  public C $y;
  public FakeType1<string> $z1;
  public FakeType2<int> $z2;
  public FakeType3<bool> $z3;
}

class B extends A {
  public function __construct() { $this->y = new C(); }
  public function test() :mixed{
    try { unset($this->y->x); } catch (Exception $e) {}
    try { unset($this->x); } catch (Exception $e) {}
    try { unset($this->z1); } catch (Exception $e) {}
    try { unset($this->z2); } catch (Exception $e) {}
    try { unset($this->z3); } catch (Exception $e) {}
  }
}
<<__EntryPoint>> function main(): void {
set_error_handler(err<>);
(new B())->test();
}
