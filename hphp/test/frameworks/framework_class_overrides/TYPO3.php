<?hh
require_once __DIR__.'/../Framework.php';

class Typo3 extends Framework {
  protected $additionalFolders = array('typo3temp', 'uploads', 'typo3conf', 'typo3conf/ext');

  public function __construct(string $name) {
    $typo3path = Options::$frameworks_root . '/typo3';
    $env_vars = Map { "TYPO3_PATH_WEB" =>  $typo3path };
    parent::__construct($name, null, $env_vars, null, false, TestFindModes::TOKEN);
  }

  protected function install(): void {
    parent::install();
    foreach($this->additionalFolders as $folder) {
      mkdir($this->getInstallRoot() . '/' . $folder);
    }
  }

  protected function isInstalled(): bool {
    foreach($this->additionalFolders as $folder) {
      if(!file_exists($this->getInstallRoot() . '/' . $folder)) {
        remove_dir_recursive($this->getInstallRoot());
        return false;
      }
    }
    return parent::isInstalled();
  }
}
