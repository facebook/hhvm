<?hh
require_once __DIR__.'/../Framework.php';

class Yii2 extends Framework {
  public function __construct(string $name) {
    $env = Map { "PHP_BINARY" =>  get_runtime_build(true) };
    $parallel = false;
    parent::__construct($name, null, $env,
	    null, $parallel, TestFindModes::TOKEN);
  }

  public function clean(): void {
    parent::clean();
    $files = glob($this->getInstallRoot().
                  "/tests/runtime/*");
    foreach ($files as $file) {
      verbose("Removing $file\n");
      unlink($file);
    }
  }
}
