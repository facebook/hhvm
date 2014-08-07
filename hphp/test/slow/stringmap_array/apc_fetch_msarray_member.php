<?hh

class Foo {
  public $msarray = msarray(
    'bar' => 'foo',
  );
}

function main() {
  $key = 'apc_key';
  $foo = new Foo;
  apc_store($key, $foo);
  $foo->msarray['bar'] = 'bar';
  var_dump($foo->msarray);

  $foo_read = apc_fetch($key);
  $foo_read->msarray['foo'] = 'hello';
  var_dump($foo_read->msarray);
}

main();
