<?hh
require_once __DIR__.'/../Framework.php';

class PHPUnit extends Framework {
  public function __construct(string $name) {
    $tc = get_runtime_build()." ".Options::$frameworks_root.
          "/phpunit/phpunit";
    $env_vars = Map { "PHP_BINARY" =>  get_runtime_build(true) };
    parent::__construct($name, $tc, $env_vars);
  }
}
