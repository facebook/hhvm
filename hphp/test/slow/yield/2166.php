<?hh

function get() :mixed{
 return true;
 }

<<__EntryPoint>>
function main_2166() :mixed{
if (get()) {
  $gen = function ($i) {
    yield $i;
    yield $i + 1;
  };
}
 else {
  $gen = function ($i) {
    yield $i + 1;
    yield $i + 2;
  };
}
foreach ($gen(3) as $x) {
 var_dump($x);
 }
}
