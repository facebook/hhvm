<?hh
function __autoload($c) {
  throw new Exception("Ha!");
}
function test() {
  try {
    array_filter(varray[1], varray['Foo', 'Bar']);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}


<<__EntryPoint>>
function main_array_filter_autoload() {
test();
}
