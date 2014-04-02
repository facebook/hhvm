<?hh
require_once __DIR__.'/../Framework.php';

class Drupal extends Framework {
  public function __construct(string $name) {
    $args_for_tests = Map {
      Options::$frameworks_root.'/drupal/core/./tests/Drupal/Tests/Core/Extension/ThemeHandlerTest.php' => '-d error_reporting=E_ERROR',
    };
    parent::__construct($name, null, null, $args_for_tests);
  }
}
