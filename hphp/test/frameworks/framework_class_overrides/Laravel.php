<?hh
require_once __DIR__.'/../Framework.php';

class Laravel extends Framework {
  public function __construct(string $name) {
    $args_for_tests = Map {
      Options::$frameworks_root."/laravel/./tests/Auth/AuthGuardTest.php" =>
      "-v JitEnableRenameFunction"
    };
    parent::__construct($name, null, null, $args_for_tests);
  }
}
