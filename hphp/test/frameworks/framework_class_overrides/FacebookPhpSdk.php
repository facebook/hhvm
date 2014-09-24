<?hh
require_once __DIR__.'/../Framework.php';

class FacebookPhpSdk extends Framework {
  <<Override>>
  protected function extraPostComposer(): void {
    verbose("Creating a phpunit.xml for running the Facebook PHP SDK tests.\n");
    $phpunit_xml = <<<XML
<phpunit bootstrap="./tests/bootstrap.php">
<testsuites>
  <testsuite name="FacebookPhpSdk">
    <directory suffix="tests.php">tests</directory>
  </testsuite>
</testsuites>
</phpunit>
XML;
    file_put_contents($this->getTestPath()."/phpunit.xml", $phpunit_xml);
  }

  protected function isInstalled(): bool {
    $extra_files = Set {
      $this->getTestPath()."/phpunit.xml",
    };

    if (file_exists($this->getInstallRoot())) {
      foreach ($extra_files as $file) {
        if (!file_exists($file)) {
          remove_dir_recursive(nullthrows($this->getInstallRoot()));
          return false;
        }
      }
    }
    return parent::isInstalled();
  }
}
