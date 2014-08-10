<?hh

class Foo {
  public $idsToNames = miarray(
    1 => 'foo',
    2 => 'bar',
  );
}

function main() {
  $foo = new Foo;
  $foo->idsToNames['warning'] = true;
  var_dump($foo->idsToNames);
  $otherFoo = new Foo;
  $otherFoo->idsToNames['still warn'] = true;
  var_dump($otherFoo->idsToNames);
}

main();
