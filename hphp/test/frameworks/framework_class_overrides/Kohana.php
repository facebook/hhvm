<?hh
require_once __DIR__.'/../Framework.php';

class Kohana extends Framework {
  public function __construct(string $name) {
    parent::__construct($name, null, null, null, true, TestFindModes::TOKEN);
  }

  protected function install(): void {
    parent::install();

    verbose("Initialize submodules.\n", Options::$verbose);
    $git_command = "git submodule update --init";
    $git_ret = run_install($git_command, $this->getInstallRoot(),
                           ProxyInformation::$proxies);
    if ($git_ret !== 0) {
      remove_dir_recursive($this->getInstallRoot());
      error_and_exit("Could not initialize submodules for ". $this->name.
                     "! Removing framework!\n", Options::$csv_only);
    }

    verbose("Updating submodule branches.\n", Options::$verbose);
    // Manually checkout release appropriate system branch
    // See: https://github.com/kohana/kohana/wiki/developers
    $git_command = 'git submodule foreach "';
    $git_command .= 'git fetch && git checkout';
    $git_command .= ' '.$this->getGitBranch().'"';
    $git_ret = run_install($git_command, $this->getInstallRoot(),
                           ProxyInformation::$proxies);
    if ($git_ret !== 0) {
      remove_dir_recursive($this->getInstallRoot());
      error_and_exit("Could not checkout submodule branch for ". $this->name.
                     "! Removing framework!\n", Options::$csv_only);
    }

    verbose("Creating a phpunit.xml for running the Kohana tests.\n",
            Options::$verbose);
    $phpunit_xml = <<<XML
<phpunit bootstrap="./modules/unittest/bootstrap_all_modules.php">
<testsuites>
  <testsuite name="Kohana">
    <directory>./system</directory>
    <directory>./modules</directory>
  </testsuite>
</testsuites>
</phpunit>
XML;
    file_put_contents($this->getTestPath()."/phpunit.xml", $phpunit_xml);
  }

  protected function isInstalled(): bool {
    $extra_files = Set {
      $this->getTestPath()."/phpunit.xml",
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
