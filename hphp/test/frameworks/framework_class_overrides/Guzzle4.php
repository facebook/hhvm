<?hh
require_once __DIR__.'/../Framework.php';

class Guzzle4 extends Framework {
  public function __construct(string $name) {
    // It spawns a server shared between the tests
    $parallel = false;
    parent::__construct($name, null, null,
                        null, $parallel, TestFindModes::TOKEN);
  }
}
