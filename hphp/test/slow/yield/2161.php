<?hh

class Env { public static int $n = 3; }

<<__EntryPoint>>
function main_2161() :mixed{
$f = function ($arg0) {
  yield $arg0;
  yield $arg0 + (Env::$n++);
  yield $arg0 + (Env::$n++) + 1;
}
;
foreach ($f(32) as $x) {
 var_dump($x);
 }
var_dump(Env::$n);
}
