<?hh
require_once __DIR__.'/../Framework.php';

class Typo3 extends Framework {
  protected $additionalFolders = array('typo3temp', 'uploads',
                                       'typo3conf', 'typo3conf/ext');

  public function __construct(string $name) {
    $typo3path = Options::$frameworks_root . '/typo3';
    $env_vars = Map { "TYPO3_PATH_WEB" =>  $typo3path };
    $tc = get_runtime_build().' '.__DIR__.
      '/../framework_downloads/typo3/bin/phpunit';
    parent::__construct($name, $tc, $env_vars, null,
                        false, TestFindModes::TOKEN);
  }

  <<__Override>>
  protected function extraPostComposer(): void {
    $rootDir = nullthrows($this->getInstallRoot());
    foreach($this->additionalFolders as $folder) {
      mkdir($rootDir . '/' . $folder);
    }
  }

  <<__Override>>
  protected function isInstalled(): bool {
    foreach($this->additionalFolders as $folder) {
      if(!file_exists($this->getInstallRoot() . '/' . $folder)) {
        if(file_exists($this->getInstallRoot())) {

          remove_dir_recursive(nullthrows($this->getInstallRoot()));
        }
        return false;
      }
    }
    return parent::isInstalled();
  }
}
