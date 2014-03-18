<?hh
require_once __DIR__.'/../Framework.php';

class Mediawiki extends Framework {
  public function __construct(string $name) {
    $tc = get_runtime_build().' --php '.Options::$frameworks_root.
          "/mediawiki-core/tests/phpunit/phpunit.php ";

    parent::__construct($name, $tc, null, null, true, TestFindModes::TOKEN);
  }

  protected function install(): void {
    parent::install();
    $this->skipDatabaseTests();
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

  // We don't have a MySQL instance running for our test runs, so Database
  // tests won't work
  private function skipDatabaseTests() {
    verbose("Modifying suite.xml to skip database tests\n", Options::$verbose);
    $file = Options::$frameworks_root.'/mediawiki-core/tests/phpunit/suite.xml';
    $doc = new DOMDocument();
    $doc->load($file);

    $database_node = $doc->createElement('group');
    $database_node->appendChild($doc->createTextNode('Database'));

    $xpath = new DOMXPath($doc);
    $exclude = $xpath->evaluate('/phpunit/groups/exclude')->item(0);
    $exclude->appendChild($database_node);

    file_put_contents($file, $doc->saveXML());
  }
}
