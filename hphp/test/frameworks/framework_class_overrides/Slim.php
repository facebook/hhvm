<?hh
require_once __DIR__.'/../Framework.php';

class Slim extends Framework {
  public function __construct(string $name) {
    $tc = get_runtime_build().' -c '.__DIR__.'/../php_notice.ini'.
          ' '.__DIR__.'/../vendor/bin/phpunit';
    parent::__construct($name, $tc);
  }
}
