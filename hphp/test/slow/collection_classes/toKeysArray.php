<?hh

<<__EntryPoint>>
function main_to_keys_array() :mixed{
$x = Vector {'a'};
var_dump($x->toKeysArray());
var_dump($x->lazy()->toKeysArray());
var_dump($x->lazy()->map(function($x){return $x;})->toKeysArray());
$x = Map {123 => 'a'};
var_dump($x->toKeysArray());
var_dump($x->lazy()->toKeysArray());
var_dump($x->lazy()->map(function($x){return $x;})->toKeysArray());
$x = Pair {'a', 'b'};
var_dump($x->toKeysArray());
var_dump($x->lazy()->toKeysArray());
var_dump($x->lazy()->map(function($x){return $x;})->toKeysArray());
}
