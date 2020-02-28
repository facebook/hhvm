<?hh

class Foo {
  private $thing;
  function go() {
    // This better not iterate the dataflow algorithm forever:
    $this->thing = darray['thing' => $this->thing];
    return $this->thing;
  }
}

function main() {
  var_dump((new Foo)->go());
}


<<__EntryPoint>>
function main_array_009() {
main();
}
