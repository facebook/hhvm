<?hh
require_once __DIR__.'/../Framework.php';

class Yii2 extends Framework {
  public function __construct(string $name) {
    parent::__construct($name, null, null, null, false);
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
