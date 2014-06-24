<?hh
require_once __DIR__.'/../Framework.php';

class Monolog extends Framework {
  public function __construct(string $name) {
    // This disables mongodb support
    $tc = get_runtime_build().' -c '.__DIR__.'/Monolog.ini'.
          ' '.__DIR__.'/../vendor/bin/phpunit';
    parent::__construct($name, $tc);
  }
}
