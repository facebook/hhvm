<?hh
require_once __DIR__.'/../Framework.php';

class Guzzle extends Framework {
  public function __construct(string $name) {
    $tc = get_runtime_build().' '.__DIR__.
      '/../framework_downloads/guzzle/vendor/bin/phpunit';
    parent::__construct($name, $tc, null, null, true, TestFindModes::TOKEN);
  }
}
