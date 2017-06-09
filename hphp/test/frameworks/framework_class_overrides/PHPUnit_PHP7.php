<?hh
require_once __DIR__.'/../Framework.php';

class PHPUnit_PHP7 extends Framework {
  public function __construct(string $name) {
    $tc = get_runtime_build().' -c '.__DIR__.'/../php7.ini'.
      ' ./phpunit';
    parent::__construct($name, $tc);
  }
}
