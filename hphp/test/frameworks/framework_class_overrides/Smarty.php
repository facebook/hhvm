<?hh
require_once __DIR__.'/../Framework.php';
require_once __DIR__.'/../utils.php';
class Smarty extends Framework {
  <<Override>>
  protected function extraPostComposer(): void {
    verbose("Moving key smarty directories out of vendor");
    rename($this->getInstallRoot()."/vendor/smarty/smarty",
           $this->getInstallRoot()."/smarty");
    rename($this->getInstallRoot()."/vendor/smarty/smarty-lexer",
           $this->getInstallRoot()."/smarty-lexer");
    rename($this->getInstallRoot()."/vendor/smarty/smarty-phpunit",
           $this->getInstallRoot()."/smarty-phpunit");
    verbose("Creating a new phpunit.xml for running the yii tests.\n");
    $phpunit_xml = file_get_contents(__DIR__ . "/smarty-phpunit.xml");
    file_put_contents($this->getTestPath()."/phpunit.xml.dist", $phpunit_xml);
    delete_file($this->getTestPath()."/phpunit.xml");
  }
}
