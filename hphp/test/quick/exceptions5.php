<?hh
class C {
  public function baz($z) :mixed{
    return $z = call_user_func(varray[$this,'foo'], $z);
  }
  public function bar($z) :mixed{
    return $z = call_user_func(varray[$this,'baz'], $z);
  }
  public function foo($z) :mixed{
    return $z = call_user_func(varray[$this,'bar'], $z);
  }
}
<<__EntryPoint>> function bar() :mixed{
  $obj = new C;
  $obj->foo(123);
}
