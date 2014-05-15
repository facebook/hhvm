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

  protected function install(): void {
    parent::install();
    foreach($this->additionalFolders as $folder) {
      mkdir($this->getInstallRoot() . '/' . $folder);
    }
    // comment out 2 methods from interfaces which couses troubles
    // because of https://github.com/facebook/hhvm/issues/2527
    $filePath = $this->getInstallRoot() .
                '/typo3/sysext/core/Resources/PHP/TYPO3.Flow/Classes/'.
                'TYPO3/Flow/Package/PackageManagerInterface.php';
    file_put_contents($filePath,
                      str_replace('public function initialize(',
                                  '//public function initialize(',
                                  file_get_contents($filePath)));
    $filePath = $this->getInstallRoot() .
                '/typo3/sysext/core/Resources/PHP/TYPO3.Flow/Classes/'.
                'TYPO3/Flow/Package/PackageInterface.php';
    file_put_contents($filePath,
                      str_replace('public function boot(',
                                  '//public function boot(',
                                  file_get_contents($filePath)));
  }

  protected function isInstalled(): bool {
    foreach($this->additionalFolders as $folder) {
      if(!file_exists($this->getInstallRoot() . '/' . $folder)) {
        if(file_exists($this->getInstallRoot())) {
          remove_dir_recursive($this->getInstallRoot());
        }
        return false;
      }
    }
    return parent::isInstalled();
  }
}
