<?hh
require_once __DIR__.'/../Framework.php';

class SilverStripe extends Framework {
  public function __construct(string $name) {
    parent::__construct($name);
  }
  protected function install(): void {
    parent::install();
    verbose("Installing dependencies.\n", Options::$verbose);

    $dependencies_install_cmd = get_runtime_build()." ".__DIR__.
      "/../composer.phar require silverstripe/sqlite3 dev-master";
    $install_ret = run_install($dependencies_install_cmd,
                               $this->getInstallRoot(),
                               ProxyInformation::$proxies);

    if ($install_ret !== 0) {
      remove_dir_recursive($this->getInstallRoot());
      error_and_exit("Couldn't download dependencies for ".$this->getName().
                     ". Removing framework. \n", Options::$csv_only);
    }

    verbose(
      "Creating a _ss_environment file for setting SQLite adapter.\n",
      Options::$verbose
    );

    $contents = <<<'ENV_FILE'
<?php
define('SS_DATABASE_SERVER', 'localhost');
define('SS_DATABASE_USERNAME', 'root');
define('SS_DATABASE_PASSWORD', '');
define('SS_DATABASE_NAME', 'tests');
define('SS_ENVIRONMENT_TYPE', 'dev');
define('SS_DATABASE_CLASS', 'SQLiteDatabase');
define('SS_DATABASE_MEMORY', true);

global $_FILE_TO_URL_MAPPING;
$_FILE_TO_URL_MAPPING[__DIR__] = 'http://localhost';
$_GET['flush'] = 1;
ENV_FILE;

    file_put_contents(
      $this->getInstallRoot()."/_ss_environment.php", $contents
    );
  }

   protected function isInstalled(): bool {
    $extra_files = Set {
      $this->getInstallRoot()."/sqlite3",
      $this->getInstallRoot()."/_ss_environment.php",
    };

    if (file_exists($this->getInstallRoot())) {
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
