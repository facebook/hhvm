<?hh

class C {
  function simpleRet($x) :mixed{
    return 1;
  }
}

function foo() :mixed{
  $x = new C;
  $y = $x->simpleRet($x);
  var_dump($x);
  var_dump($y);
}
<<__EntryPoint>> function main(): void {
foo();

echo "End\n";
}
