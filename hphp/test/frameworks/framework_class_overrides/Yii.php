<?hh
require_once __DIR__.'/../Framework.php';

class Yii extends Framework {
  public function __construct(string $name) {
    $env_vars = Map { "PHP_BINARY" =>  get_runtime_build(true) };
    parent::__construct($name, null, $env_vars);
  }
  public function clean(): void {
    parent::clean();
    $files = glob($this->getInstallRoot().
                  "/tests/assets/*/CAssetManagerTest.php");
    foreach ($files as $file) {
      verbose("Removing $file\n", Options::$verbose);
      unlink($file);
    }
  }

  protected function install(): void {
    parent::install();
    verbose("Creating a new phpunit.xml for running the yii tests.\n",
            Options::$verbose);
    $phpunit_xml = <<<XML
<phpunit bootstrap="bootstrap.php"
    colors="false"
    convertErrorsToExceptions="true"
    convertNoticesToExceptions="true"
    convertWarningsToExceptions="true"
    stopOnFailure="false">
<testsuites>
  <testsuite name="yii">
    <directory suffix="Test.php">./</directory>
  </testsuite>
</testsuites>
</phpunit>
XML;
    file_put_contents($this->getTestPath()."/phpunit.xml.dist", $phpunit_xml);
    unlink($this->getTestPath()."/phpunit.xml");
  }

  protected function isInstalled(): bool {
    $extra_files = Set {
      $this->getTestPath()."/phpunit.xml.dist",
    };

    if (file_exists($this->getInstallRoot())) {
      // Make sure all the pull requests that have been added along the way
      // are there; otherwise we need a redownload.
      foreach ($extra_files as $file) {
        if (!file_exists($file)) {
          remove_dir_recursive($this->getInstallRoot());
          return false;
        }
      }
    }
    return parent::isInstalled();
  }
}
