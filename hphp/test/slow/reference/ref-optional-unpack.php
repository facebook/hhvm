<?hh

function foo($bar, &$baz=null) {
  $baz = $bar;
}

$val = null;
foo(42, &$val);
var_dump($val);

$arr = array();
foo(43, ...$arr);
var_dump($arr);

$arr = array($val);
foo(44, ...$arr);
var_dump($val);
var_dump($arr);
