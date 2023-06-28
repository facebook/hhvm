<?hh

function favorite_fruit() :AsyncGenerator<mixed,mixed,void>{
  yield 'Tim' => 'apple';
  yield 'Bob' => 'pear';
}


<<__EntryPoint>>
function main_2228() :mixed{
foreach (favorite_fruit() as $person => $fruit) {
  var_dump($person);
  var_dump($fruit);
}
}
