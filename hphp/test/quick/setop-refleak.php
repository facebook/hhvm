<?hh

class Dtor {
}

class Foo { public $bug; }

function main() {
  $x = new Foo;
  $x->bug = new Dtor;
  try {
    $x->bug += 12;
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
  var_dump($x);
}
<<__EntryPoint>> function main_entry() {
main();
echo "done\n";
}
