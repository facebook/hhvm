<?hh
require_once __DIR__.'/../Framework.php';

class Mediawiki extends Framework {
  public function __construct(string $name) {
    $tc = get_runtime_build().' --php '.Options::$frameworks_root.
          "/mediawiki-core/tests/phpunit/phpunit.php ".
          "--exclude-group=Database,Broken ";
    parent::__construct($name, $tc, null, null, true, TestFindModes::TOKEN);
  }

  protected function install(): void {
    parent::install();
    # Mediawiki has a custom test runner that pulls in parts of PHPUnit
    # from vendor/
    verbose("Installing phpunit 3.7 for Mediawiki.\n", Options::$verbose);
    $phpunit = get_runtime_build().' '.__DIR__.
      '/../composer.phar --working-dir='.$this->getInstallRoot().' '.
      "require 'phpunit/phpunit=3.7.*'";
    exec($phpunit);
    verbose("Adding LocalSettings.php file to Mediawiki test dir.\n",
            Options::$verbose);
    $touch_command = "touch ".$this->getInstallRoot()."/LocalSettings.php";
    exec($touch_command);
  }
}
