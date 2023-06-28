<?hh

class Ref {
  function __construct(public $value)[] {}
}
function gen($x) :AsyncGenerator<mixed,mixed,void>{
  $x->value = 1;
  yield 1;
  $x->value = 2;
  yield 3;
  $x->value = 4;
}
function test() :mixed{
  $x = new Ref(0);
  foreach (gen($x) as $y) {
    var_dump($y);
  }
  var_dump($x->value);
}

<<__EntryPoint>>
function main_2186() :mixed{
test();
}
