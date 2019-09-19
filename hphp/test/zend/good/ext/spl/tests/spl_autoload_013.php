<?hh

class Autoloader {
  private $dir;
  public function __construct($dir) {
    $this->dir = $dir;
  }
  public function __invoke($class) {
    var_dump("{$this->dir}/$class.php");
  }
}
<<__EntryPoint>>
function main_entry(): void {
  $closure = function($class) {
    echo "a called\n";
  };

  $al1 = new Autoloader('d1');
  $al2 = new Autoloader('d2');

  spl_autoload_register($closure);
  spl_autoload_register($al1);
  spl_autoload_register($al2);

  var_dump(spl_autoload_functions());

  echo "===DONE===\n";
}
