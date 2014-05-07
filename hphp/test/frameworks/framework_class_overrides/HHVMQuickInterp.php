<?hh
require_once __DIR__.'/../Framework.php';

class HHVMQuickInterp extends Framework {
  public function __construct(string $name) {
    $parallel = false;
    parent::__construct($name, null, null,
                        null, $parallel, TestFindModes::TOKEN);
  }
}
