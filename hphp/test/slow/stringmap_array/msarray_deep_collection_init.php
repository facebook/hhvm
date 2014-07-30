<?hh

class Foo {
  public $hodgePodge = Vector {
    msarray(),
    Vector {
      1,
      2,
      3,
    },
  };
}

function main() {
  $foo = new Foo;
  $foo->hodgePodge[0][-10] = 'warning';
  var_dump($foo->hodgePodge);
  $otherFoo = new Foo;
  $otherFoo->hodgePodge[0][99] = 'still warn';
  var_dump($foo->hodgePodge);
}

main();
