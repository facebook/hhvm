<?hh
require_once __DIR__.'/../Framework.php';

class Kohana extends Framework {
  protected function install(): void {
    parent::install();
    $root = nullthrows($this->getInstallRoot());

    verbose("Initialize submodules.\n");
    $git_command = "git submodule update --init";
    $git_ret = run_install($git_command, $root,
                           ProxyInformation::$proxies);
    if ($git_ret !== 0) {
      remove_dir_recursive($root);
      error_and_exit("Could not initialize submodules for ". $this->name.
                     "! Removing framework!\n");
    }

    verbose("Updating submodule branches.\n");
    // Manually checkout release appropriate system branch
    // See: https://github.com/kohana/kohana/wiki/developers
    $git_command = 'git submodule foreach "';
    $git_command .= 'git fetch && git checkout';
    $git_command .= ' '.$this->getGitBranch().'"';
    $git_ret = run_install($git_command, $root,
                           ProxyInformation::$proxies);
    if ($git_ret !== 0) {
      remove_dir_recursive($root);
      error_and_exit("Could not checkout submodule branch for ". $this->name.
                     "! Removing framework!\n");
    }

    verbose("Creating a phpunit.xml for running the Kohana tests.\n");
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
    $root = nullthrows($this->getInstallRoot());

    if (file_exists($root)) {
      foreach ($extra_files as $file) {
        if (!file_exists($file)) {
          remove_dir_recursive($root);
          return false;
        }
      }
    }
    return parent::isInstalled();
  }
}
