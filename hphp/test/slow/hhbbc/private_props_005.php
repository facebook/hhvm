<?hh

class UnknownUnsetThis {
  private $s = "this is a string";
  private $i = 2;

  public function __construct(string $l) {
    unset($this->{$l});
  }

  public function printer() {
    var_dump(isset($this->s), isset($this->i));
  }
}

function main() {
  $x = new UnknownUnsetThis('s');
  $x->printer();
  $x = new UnknownUnsetThis('i');
  $x->printer();
}

main();
