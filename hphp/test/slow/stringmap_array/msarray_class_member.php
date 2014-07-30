<?hh

class Foo {
  public $namesToIds = msarray(
    'foo' => 23409,
    'bar' => 23499949,
  );
}

function main() {
  $foo = new Foo;
  $foo->namesToIds[1] = 'warning';
  var_dump($foo->namesToIds);
  $otherFoo = new Foo;
  $otherFoo->namesToIds[20] = 'still warning';
  var_dump($otherFoo->namesToIds);
}

main();
