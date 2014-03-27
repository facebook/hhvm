<?hh
require_once __DIR__.'/../Framework.php';

class Pear extends Framework {
  public function __construct(string $name) {
    // Pear will currently run serially
    parent::__construct($name, null, null, null, false, TestFindModes::PHPT);
  }

  protected function install(): void {
    parent::install();
    verbose("Creating a bootstrap.php for running the pear tests.\n",
            Options::$verbose);
    $bootstrap_php = <<<BOOTSTRAP
<?php
putenv('PHP_PEAR_RUNTESTS=1');
BOOTSTRAP;
    file_put_contents($this->getTestPath()."/bootstrap.php", $bootstrap_php);
    verbose("Creating a phpunit.xml for running the pear tests.\n",
            Options::$verbose);
    $phpunit_xml = <<<XML
<phpunit bootstrap="bootstrap.php">
<testsuites>
  <testsuite name="Pear">
    <directory suffix=".phpt">tests</directory>
  </testsuite>
</testsuites>
</phpunit>
XML;
    file_put_contents($this->getTestPath()."/phpunit.xml", $phpunit_xml);
  }

  protected function isInstalled(): bool {
    $extra_files = Set {
      $this->getTestPath()."/phpunit.xml",
      $this->getInstallRoot()."/Console",
      $this->getInstallRoot()."/XML",
      $this->getInstallRoot()."/Structures",
      $this->getInstallRoot()."/Archive",
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
