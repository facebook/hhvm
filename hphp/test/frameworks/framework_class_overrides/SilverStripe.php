<?hh
require_once __DIR__.'/../Framework.php';

class SilverStripe extends Framework {
  public function __construct(string $name) {
    parent::__construct($name);
  }

  protected function installDependencies(): void {
    $composer_json_path = find_first_file_recursive(
      Set {'composer.json'},
      nullthrows($this->getInstallRoot()),
      true
    );
    if ($composer_json_path !== null) {
      $composer_config = json_decode(
        file_get_contents($composer_json_path.'/composer.json'),
        /* assoc = */ true
      );
      // Add in SQLite module
      $composer_config['require']['silverstripe/sqlite3'] = '1.3.x-dev';
      // Change existing dependencies to the wanted tag
      $composer_config['require']['silverstripe/cms'] = '3.1.5';
      $composer_config['require']['silverstripe/framework'] = '3.1.5';
      file_put_contents(
        $composer_json_path.'/composer.json',
        json_encode($composer_config)
      );
    }
    parent::installDependencies();
  }

  protected function install(): void {
    parent::install();

    verbose("Creating a _ss_environment file for setting SQLite adapter.\n");

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
          remove_dir_recursive(nullthrows($this->getInstallRoot()));
          return false;
        }
      }
    }
    return parent::isInstalled();
  }
}
