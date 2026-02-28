<?hh


<<__EntryPoint>>
function main_comparison() :mixed{
$ca = function() {};
$cb = function($a) { return $a; };

var_dump($ca == $cb);
var_dump(HH\Lib\Legacy_FIXME\lt($ca, $cb));
var_dump(HH\Lib\Legacy_FIXME\gt($ca, $cb));
var_dump(HH\Lib\Legacy_FIXME\lte($ca, $cb));
var_dump(HH\Lib\Legacy_FIXME\gte($ca, $cb));
}
