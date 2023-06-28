<?hh

function foo() :mixed{
 return "asd";
 }
function bar() :mixed{
 return "bar";
 }

class Bar {
  public function asd() :mixed{
 return $this;
 }
}

class Baz {
  public function k() :mixed{
 return 12;
 }
}

function main() :mixed{
  $k = new Bar;
  $y = new Baz;
  foo();
  $k->asd();
  $y->k();
}

function const_fold() :mixed{
  echo foo().bar()."\n";
}


<<__EntryPoint>>
function main_constants() :mixed{
main();
const_fold();
}
