<?hh

function get() {
 return true;
 }

<<__EntryPoint>>
function main_1939() {
if (get()) {
  $g = function ($x) {
    return function () use ($x) {
 return $x;
 }
;
  };
}
 else {
  $g = function ($x) {
    return function () use ($x) {
 return $x + 1;
 }
;
  };
}
$f = $g(32);
var_dump($f());
}
