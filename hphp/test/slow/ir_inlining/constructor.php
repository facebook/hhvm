<?hh

class foo {
  private $prop;
  public function __construct($s) {
    $this->prop = $s;
  }
}

function main() {
  new foo('hi');
}

<<__EntryPoint>>
function main_constructor() {
main();
echo "done\n";
}
