<?hh
require_once __DIR__.'/../Framework.php';

class PHPUnit_PHP7 extends Framework {
  public function __construct(string $name) {
    $tc = get_runtime_build().' ./phpunit';
    parent::__construct($name, $tc);
  }
}
