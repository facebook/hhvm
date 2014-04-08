<?hh
require_once __DIR__.'/../Framework.php';

class Guzzle extends Framework {
  public function __construct(string $name) {
    $tc = get_runtime_build().' '.__DIR__.
      '/../framework_downloads/guzzle/vendor/bin/phpunit';
    // PHPUnit <4.x @runInSeparateProcess fix
    $env = Map { "PHP_BINARY" =>  get_runtime_build(true) };
    $parallel = false;
    parent::__construct($name, $tc, $env,
                        null, $parallel, TestFindModes::TOKEN);
  }
}
