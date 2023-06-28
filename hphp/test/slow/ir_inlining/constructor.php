<?hh

class foo {
  private $prop;
  public function __construct($s) {
    $this->prop = $s;
  }
}

function main() :mixed{
  new foo('hi');
}

<<__EntryPoint>>
function main_constructor() :mixed{
main();
echo "done\n";
}
