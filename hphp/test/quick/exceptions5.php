<?hh
class C {
  public function baz($z) {
    return $z = call_user_func(varray[$this,'foo'], $z);
  }
  public function bar($z) {
    return $z = call_user_func(varray[$this,'baz'], $z);
  }
  public function foo($z) {
    return $z = call_user_func(varray[$this,'bar'], $z);
  }
}
<<__EntryPoint>> function bar() {
  $obj = new C;
  $obj->foo(123);
}
