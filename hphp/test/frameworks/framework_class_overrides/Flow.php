<?hh
require_once __DIR__.'/../Framework.php';

class Flow extends Framework {

  public function __construct(string $name) {
    //we need newer phpunit 4.5
    $tc = get_runtime_build().' '.__DIR__.
      '/../framework_downloads/flow/bin/phpunit';
    parent::__construct($name, $tc); //, null, null, false);
  }
}
