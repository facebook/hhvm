<?hh

class Foo {
  public $hodgePodge = Vector {
    miarray(),
    Vector {
      1,
      2,
      3,
    },
  };
}

function main() {
  $foo = new Foo;
  $foo->hodgePodge[0]['warning'] = true;
  var_dump($foo->hodgePodge);
  $otherFoo = new Foo;
  $otherFoo->hodgePodge[0]['still warn'] = true;
  var_dump($foo->hodgePodge);
}

main();
