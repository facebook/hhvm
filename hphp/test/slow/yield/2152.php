<?hh

class Ref {
  function __construct(public $value)[] {}
}
function f($a1, $a2) :AsyncGenerator<mixed,mixed,void>{
  foreach ($a1 as $k1 => $v1) {
    foreach ($a2->value as $k2 => $v2) {
      $a2->value[$k2] += $v1;
      yield $a2->value[$k2];
    }
  }
}

<<__EntryPoint>>
function main_2152() :mixed{
$a1 = vec[1, 2];
$a2 = new Ref(vec[1, 2]);
foreach (f($a1, $a2) as $v) {
 var_dump($v);
 }
var_dump($a2->value[0], $a2->value[1]);
}
