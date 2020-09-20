<?hh

function fruit() {
  $a = 123;
  yield $a;
  return;
  yield ++$a;
}



<<__EntryPoint>>
function main_2146() {
foreach (fruit() as $fruit) {
  var_dump($fruit);
}
}
