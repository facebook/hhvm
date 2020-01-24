<?hh


<<__EntryPoint>>
function main_incomplete_class() {
  // unserialize will produce __PHP_Incomplete_Class
  $incomplete = unserialize('O:8:"IGNOREME":0:{}');

  $o = new SplObjectStorage();
  $o->attach($incomplete);
  var_dump($o->count());

  $arr = new ArrayIterator($incomplete); // No Exception
}
