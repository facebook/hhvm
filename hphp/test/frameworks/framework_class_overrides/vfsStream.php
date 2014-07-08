<?hh
require_once __DIR__.'/../Framework.php';

class VfsStream extends Framework {
  public function __construct(string $name) {
    parent::__construct($name);
    $this->disableCustomPHPUnitOutput();
  }

  private function disableCustomPHPUnitOutput(): void {
    verbose(
      "Modifying phpunit.xml.dist to use standard phpunit result printer\n");
    $file = Options::$frameworks_root.'/vfsstream/phpunit.xml.dist';
    $doc = new DOMDocument();
    $doc->load($file);
    $phpunit = $doc->documentElement;
    if ($phpunit->hasAttribute('printerClass')) {
      $phpunit->removeAttribute('printerClass');
      file_put_contents($file, $doc->saveXML());
    }
  }
}
