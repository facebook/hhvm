<?hh
require_once __DIR__.'/../Framework.php';

class Mustache extends Framework {
  protected function install(): void {
    parent::install();

    verbose("Initialize submodules.\n");
    $git_command = "git submodule update --init";
    $git_ret = run_install($git_command, nullthrows($this->getInstallRoot()),
                           ProxyInformation::$proxies);
    if ($git_ret !== 0) {
      remove_dir_recursive(nullthrows($this->getInstallRoot()));
      error_and_exit("Could not initialize submodules for ". $this->name.
                     "! Removing framework!\n");
    }
  }
}
