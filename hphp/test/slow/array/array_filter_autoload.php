<?hh

function test() {
  try {
    array_filter(varray[1], varray['Foo', 'Bar']);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_array_filter_autoload() {
HH\autoload_set_paths(
  dict[
    'failure' => (...$_args) ==> { throw new Exception('Ha!'); },
  ],
  __DIR__.'/',
);

test();
}
