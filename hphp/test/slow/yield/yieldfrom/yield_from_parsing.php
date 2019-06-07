<?hh

function yieldfrom() {
  yield 42;
}


<<__EntryPoint>>
function main_yield_from_parsing() {
$g = yieldfrom();

foreach($g as $val) { var_dump($val); }
}
