<?hh

// implicitly weak mode code

function to_int(inout int $x) :mixed{}
function to_float(inout float $x) :mixed{}
function to_string(inout string $x) :mixed{}
function to_bool(inout bool $x) :mixed{}
<<__EntryPoint>> function main(): void {
$x = 1.0;
var_dump($x);
to_int(inout $x); // because $x is by-reference, the weak type hint converts it
var_dump($x);
to_float(inout $x);
var_dump($x);
to_string(inout $x);
var_dump($x);
to_bool(inout $x);
var_dump($x);
}
