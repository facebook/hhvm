<?hh

class C {
  function simpleRet() {
    return 1;
  }
}

function foo() {
  $x = new C;
  $y = $x->simpleRet();
  var_dump($x);
  var_dump($y);
}
<<__EntryPoint>> function main(): void {
foo();

echo "End\n";
}
