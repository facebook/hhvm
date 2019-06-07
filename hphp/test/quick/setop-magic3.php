<?hh

class Foo {
  public function __get($x) {
    var_dump("getter: " . $x);
    if ($x == 'foo') return 12;
    $this->foo += 12;
    $this->asd = new stdclass;
  }
}

function main() {
  $x = new Foo;
  $x->asd += 12;
  var_dump($x);
}
<<__EntryPoint>> function main_entry() {
main();
echo "Done\n";
}
