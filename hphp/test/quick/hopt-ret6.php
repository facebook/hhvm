<?hh

class C {
  function simpleRet() :mixed{
    return 1;
  }
}

function foo() :mixed{
  $x = new C;
  $y = $x->simpleRet();
  var_dump($x);
  var_dump($y);
}
<<__EntryPoint>> function main(): void {
foo();

echo "End\n";
}
