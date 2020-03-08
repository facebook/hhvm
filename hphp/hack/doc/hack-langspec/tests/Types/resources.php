<?hh // strict

namespace NS_resources;

class C {
  const resource INFILE = STDIN;
  private resource $prop; // = STDOUT;   // causes segmentation fault

  public function __construct() {
    $this->prop = STDOUT;
  }

  public function setProp(resource $val): void {
    $this->prop = $val;
  }

  public function getProp(): resource {
    return $this->prop;
  }
}

function main(): void {
  var_dump(STDIN);
  var_dump(is_resource(STDIN));
  var_dump(get_resource_type(STDIN));

  var_dump(STDOUT);
  var_dump(is_resource(STDIN));
  var_dump(get_resource_type(STDIN));

  var_dump(STDERR);
  var_dump(is_resource(STDIN));
  var_dump(get_resource_type(STDIN));

  $infile = fopen("Testfile.txt", 'r');
  var_dump($infile);
  echo "\n";
  print_r($infile);
  echo "\n";

  $infile = fopen("NoSuchFile.txt", 'r');
  var_dump($infile);

  $c = new C();
  var_dump($c);
}

/* HH_FIXME[1002] call to main in strict*/
main();

