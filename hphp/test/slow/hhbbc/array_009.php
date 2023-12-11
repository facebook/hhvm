<?hh

class Foo {
  private $thing;
  function go() :mixed{
    // This better not iterate the dataflow algorithm forever:
    $this->thing = dict['thing' => $this->thing];
    return $this->thing;
  }
}

function main() :mixed{
  var_dump((new Foo)->go());
}


<<__EntryPoint>>
function main_array_009() :mixed{
main();
}
