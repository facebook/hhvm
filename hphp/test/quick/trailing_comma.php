<?hh

function id($x,) :mixed{return $x;}

function multiline(
  $x,
  $y,
) :mixed{ return $x+$y; }

<<__EntryPoint>> function main(): void {
var_dump(id(1,));

var_dump(multiline(
  1,
  2,
));

$x = 3;
$y = 4;
$c = function () use (
  $x,
  $y,
) { return $x+$y; };

var_dump($c());
}
