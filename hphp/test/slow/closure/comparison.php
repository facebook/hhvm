<?hh


<<__EntryPoint>>
function main_comparison() {
$ca = function() {};
$cb = function($a) { return $a; };

var_dump($ca == $cb);
var_dump($ca < $cb);
var_dump($ca > $cb);
var_dump($ca <= $cb);
var_dump($ca >= $cb);
}
