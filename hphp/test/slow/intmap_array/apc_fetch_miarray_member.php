<?hh

class Foo {
  public $miarray = miarray(
    1 => 'foo',
  );
}

function main() {
  $key = 'apc_key';
  $foo = new Foo;
  apc_store($key, $foo);
  $foo->miarray[1] = 'bar';
  var_dump($foo->miarray);

  $foo_read = apc_fetch($key);
  $foo_read->miarray[2] = 'hello';
  var_dump($foo_read->miarray);
}

main();
