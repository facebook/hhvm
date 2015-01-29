<?hh
require_once __DIR__.'/../Framework.php';

class Symfony extends Framework {
  public function __construct(string $name) {
    $env_vars = Map { "PHP_BINARY" =>  get_runtime_build(true) };
    parent::__construct($name, null, $env_vars, null, true,
                        TestFindModes::TOKEN);
  }

  <<Override>>
  protected function extraPreComposer() {
    // Add a default timezone, because Symfony requires a
    // default timezone in the default php.ini
    verbose("Adding default timezone to autoload.php.dist");
    $default_timezone_string =
      "date_default_timezone_set('America/Los_Angeles');";
    $file = Options::$frameworks_root."/symfony/autoload.php.dist";
    $contents = explode("\n", file_get_contents($file));
    $contents[1] = $default_timezone_string;
    file_put_contents($file, implode("\n", $contents));
  }
}
