<?hh

class Dtor {
}

class Foo { public $bug; }

function main() :mixed{
  $x = new Foo;
  $x->bug = new Dtor;
  try {
    $x->bug += 12;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($x);
}
<<__EntryPoint>> function main_entry() :mixed{
main();
echo "done\n";
}
