<?hh

<<__EntryPoint>>
function main_to_values_array() :mixed{
$x = Vector {'a'};
var_dump($x->toValuesArray());
var_dump($x->lazy()->toValuesArray());
var_dump($x->lazy()->map(function($x){return $x;})->toValuesArray());
$x = Map {123 => 'a'};
var_dump($x->toValuesArray());
var_dump($x->lazy()->toValuesArray());
var_dump($x->lazy()->map(function($x){return $x;})->toValuesArray());
$x = Set {'a'};
var_dump($x->toValuesArray());
var_dump($x->lazy()->toValuesArray());
var_dump($x->lazy()->map(function($x){return $x;})->toValuesArray());
$x = Pair {'a', 'b'};
var_dump($x->toValuesArray());
var_dump($x->lazy()->toValuesArray());
var_dump($x->lazy()->map(function($x){return $x;})->toValuesArray());
}
