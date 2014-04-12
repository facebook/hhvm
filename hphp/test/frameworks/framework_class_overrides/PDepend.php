<?hh
require_once __DIR__.'/../Framework.php';

class PDepend extends Framework {
  public function __construct(string $name) {
    parent::__construct($name, null, null, null, true, TestFindModes::TOKEN);
  }
}
