<?hh
class C {
  public function baz($z) :mixed{
    $x = 1;
    return $z = call_user_func(varray[$this,'foo'], $z);
  }
  public function bar($z) :mixed{
    $x = 1;
    return $z = call_user_func(varray[$this,'baz'], $z);
  }
  public function foo($z) :mixed{
    $x = 1;
    return $z = call_user_func(varray[$this,'bar'], $z);
  }
}
<<__EntryPoint>> function bar() :mixed{
  $obj = new C;
  $obj->foo(123);
}
