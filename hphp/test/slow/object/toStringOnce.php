<?hh

class A {
  private $count = 0;
  public function __toString() :mixed{
    return (string) $this->count++;
  }
}

function main() :mixed{
  $a = new A;
  var_dump((string)$a);
  var_dump((string)$a);
  var_dump((string) (new A));
}

<<__EntryPoint>>
function main_to_string_once() :mixed{
main();
}
