<?hh
require_once __DIR__.'/../Framework.php';

class Symfony extends Framework {
  public function __construct(string $name) {
    $env_vars = Map { "PHP_BINARY" =>  get_runtime_build(true) };
    parent::__construct($name, null, $env_vars, null, true,
                        TestFindModes::TOKEN);
  }
}
