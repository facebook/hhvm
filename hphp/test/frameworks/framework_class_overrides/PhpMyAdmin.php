<?hh
require_once __DIR__.'/../Framework.php';

class PhpMyAdmin extends Framework {
  public function __construct(string $name) {
    parent::__construct(
      $name,
      null,
      null,
      null,
      /* parralel = */ false,
      TestFindModes::TOKEN
    );
  }
}
