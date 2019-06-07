<?hh

function favorite_fruit() {
  yield 'Tim' => 'apple';
  yield 'Bob' => 'pear';
}


<<__EntryPoint>>
function main_2228() {
foreach (favorite_fruit() as $person => $fruit) {
  var_dump($person);
  var_dump($fruit);
}
}
