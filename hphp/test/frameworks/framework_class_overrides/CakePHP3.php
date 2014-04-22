<?hh
require_once __DIR__.'/../Framework.php';

class CakePHP3 extends Framework {
  public function __construct(string $name) {
    $parallel = true;
    parent::__construct($name, null, null,
                        null, $parallel, TestFindModes::TOKEN);
  }
}
