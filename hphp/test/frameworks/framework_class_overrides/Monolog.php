<?hh
require_once __DIR__.'/../Framework.php';

class Monolog extends Framework {
  public function __construct(string $name) {
    // This disables mongodb support
    $tc = get_runtime_build().' -vEval.EnableZendCompat=false'.
          ' '.__DIR__.'/../vendor/bin/phpunit';
    parent::__construct($name, $tc);
  }
}
