<?hh

function fruit() :AsyncGenerator<mixed,mixed,void>{
  $a = 123;
  yield $a;
  return;
  yield ++$a;
}



<<__EntryPoint>>
function main_2146() :mixed{
foreach (fruit() as $fruit) {
  var_dump($fruit);
}
}
