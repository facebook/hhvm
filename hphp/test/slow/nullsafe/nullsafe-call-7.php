<?hh
class C {
  public function foo() :mixed{
    echo "foo() was called\n";
    return $this;
  }
  public function bar() :mixed{
    echo "bar() was called\n";
    return $this;
  }
}
function f() :mixed{
  echo "f() was called\n";
}
function g() :mixed{
  echo "g() was called\n";
}
function main() :mixed{
  $obj = new C();
  $x = $obj?->foo(f())?->bar(g());
  var_dump($x);
  $obj = null;
  $x = $obj?->foo(f())?->bar(g());
  var_dump($x);
  echo "Done\n";
}

<<__EntryPoint>>
function main_nullsafe_call_7() :mixed{
main();
}
