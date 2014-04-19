<?hh
require_once __DIR__.'/Options.php';
require_once __DIR__.'/utils.php';
require_once __DIR__.'/TestFindModes.php';

class Framework {
  private string $out_file;
  private string $expect_file;
  private string $diff_file;
  private string $errors_file;
  private string $fatals_file;
  private string $stats_file;
  private string $tests_file;
  private string $test_files_file;

  private string $test_path;
  private string $test_name_pattern;
  private string $test_file_pattern;
  private ?Map $current_test_statuses = null;
  private Set $test_files = null;

  private string $install_root;
  private string $git_path;
  private string $git_commit;
  private string $git_branch;
  private Set $blacklist;
  private Set $clownylist;
  private array<int, array<string, string>> $pull_requests;
  private Set $individual_tests = null;
  private string $bootstrap_file = null;
  private string $config_file = null;

  // $name, $parallel, $test_fine_mode, etc. are constructor promoted
  // Assume the framework unit tests will be run in parallel until otherwise
  // proven. If $parallel is set to false, then the framework will be run
  // in a similar vain as a normal PHPUnit run, it is one test after another
  // in the same PHPUnit process. Also assume that tests will be found by
  // reflecting over the framework. However, some require that we use php
  // tokens or are found via phpt files.
  public function __construct(private string $name,
                              private string $test_command = null,
                              private Map $env_vars = null,
                              private Map $args_for_tests = null,
                              private bool $parallel = true,
                              private string $test_find_mode =
                                TestFindModes::REFLECTION) {

    // Get framework information and set all needed properties. Beyond
    // the install root, git info, test roots, etc., the other
    // properties are optional and may or may not be set
    if (!array_key_exists("install_root", Options::$framework_info[$name]) ||
        !array_key_exists("test_root", Options::$framework_info[$name]) ||
        !array_key_exists('url', Options::$framework_info[$name]) ||
        !array_key_exists('commit', Options::$framework_info[$name]) ||
        !array_key_exists('branch', Options::$framework_info[$name])) {
      throw new Exception("Provide install, git and test file search info");
    }

    if (Options::$as_phpunit) {
      $this->parallel = false;
    }

    // Set Framework information for install. These are the five necessary
    // properties for a proper install, with pull_requests being optional.
    $this->setInstallRoot(Options::$framework_info[$name]["install_root"]);
    $this->setGitPath(Options::$framework_info[$name]['url']);
    $this->setGitCommit(Options::$framework_info[$name]['commit']);
    $this->setGitBranch(Options::$framework_info[$name]['branch']);
    $this->setTestPath(Options::$framework_info[$name]["test_root"]);
    $this->setPullRequests(Options::getFrameworkInfo($name, "pull_requests"));
    $this->setBlacklist(Options::getFrameworkInfo($name, "blacklist"));
    $this->setClownylist(Options::getFrameworkInfo($name, "clowns"));
    $this->setTestNamePattern(Options::getFrameworkInfo($name,
                                                        "test_name_regex"));
    $this->setTestFilePattern(Options::getFrameworkInfo($name,
                                                        "test_file_regex"));

    $this->prepareOutputFiles();

    // Install if not already installed using the properties set above.
    if (!$this->isInstalled()) {
      // This will disable tests too upon install.
      $this->install();
    } else {
      // Even if we are found out to alreay be installed, still ensure that
      // appropriate tests are disabled.
      $this->disableTestFiles();
    }

    // Now that we have an install, we can safely set all possible
    // other framework information
    $this->setConfigFile(Options::getFrameworkInfo($name, "config_file"));
    $this->setBootstrapFile(Options::getFrameworkInfo($name, "bootstrap_file"));
    $this->setTestCommand(true);
    $this->findTests();
  }

  //********************
  // Public setters
  //********************

  //********************
  // Public getters
  //********************
  public function getName(): string {
    return $this->name;
  }

  public function isParallel(): bool {
    return $this->parallel;
  }

  public function getOutFile(): string {
    return $this->out_file;
  }

  public function getFatalsFile(): string {
    return $this->fatals_file;
  }

  public function getDiffFile(): string {
    return $this->diff_file;
  }

  public function getStatsFile(): string {
    return $this->stats_file;
  }

  public function getExpectFile(): string {
    return $this->expect_file;
  }

  public function getErrorsFile(): string {
    return $this->errors_file;
  }

  public function getTestPath(): string {
    return $this->test_path;
  }

  public function getTestNamePattern(): string {
    return $this->test_name_pattern;
  }

  public function getTests(): ?Set {
    return Options::$test_by_single_test
      ? $this->individual_tests
      : $this->test_files;
  }

  public function getEnvVars(): ?Map {
    return $this->env_vars;
  }

  public function getCurrentTestStatuses(): ?Map {
    return $this->current_test_statuses;
  }

  public function getTestCommand(string $test): string {
    $command = '';
    if ($this->env_vars !== null) {
      foreach($this->env_vars as $var => $val) {
        $command .= "export ".$var."=\"".$val."\" && ";
      }
    }

    $command .= str_replace("%test%", $test, $this->test_command);
    // Replace any \ with \\ in order to run via --filter
    // method in phpunit
    $command = str_replace("\\", "\\\\", $command);
    if ($this->args_for_tests !== null) {
      $args = $this->args_for_tests->get($test);
      if ($args) {
        $command = preg_replace('#/hhvm #', '/hhvm '.$args.' ', $command);
      }
    }

    return $command;
  }

  //********************
  // Protected getters
  //********************
  protected function getInstallRoot(): string {
    return $this->install_root;
  }

  protected function getGitBranch(): string {
    return $this->git_branch;
  }

  //********************
  // Private setters
  //********************
  private function setGitPath(string $git_path): void {
    $this->git_path = $git_path;
  }

  private function setGitCommit(string $git_commit): void {
    $this->git_commit = $git_commit;
  }

  private function setGitBranch(string $git_branch): void {
    $this->git_branch = $git_branch;
  }

  private function setBlacklist(?array<int, string> $blacklist): void {
    $this->blacklist = null;
    if ($blacklist !== null) {
      $this->blacklist = Set {};
      foreach ($blacklist as $test) {
        $this->blacklist[] = Options::$frameworks_root."/".$test;
      }
    }
  }

  private function setClownylist(?array<int, string> $clownylist): void {
    $this->clownylist = null;
    if ($clownylist !== null) {
      $this->clownylist = Set {};
      foreach ($clownylist as $test) {
        $this->clownylist[] = Options::$frameworks_root."/".$test;
      }
    }
  }

  private function setPullRequests(
    ?array<int, array<string, string>> $pull_requests
    ): void {
    $this->pull_requests = null;
    if ($pull_requests !== null) {
      $this->pull_requests = array();
      foreach($pull_requests as $pr) {
        if (array_key_exists("pull_dir", $pr)) {
          $pr['pull_dir'] = Options::$frameworks_root."/".$pr['pull_dir'];
        }
        if (array_key_exists("move_from_dir", $pr)) {
          $pr['move_from_dir'] = Options::$frameworks_root."/".
                                 $pr['move_from_dir'];
        }
        if (array_key_exists("dir_to_move", $pr)) {
          $pr['dir_to_move'] = Options::$frameworks_root."/".$pr['dir_to_move'];
        }
        $this->pull_requests[] = $pr;
      }
    }
  }

  private function setBootstrapFile(?string $bootstrap_file): void {
    $this->bootstrap_file = $bootstrap_file;
  }

  private function setTestPath(string $test_path): void {
    $this->test_path = Options::$frameworks_root."/".$test_path;
  }

  private function setInstallRoot(string $install_root): void {
    $this->install_root = Options::$frameworks_root."/".$install_root;
  }

  private function setTestNamePattern(?string $test_name_pattern): void {
    // Test name pattern can be different depending on the framework,
    // although most follow the default.
    $this->test_name_pattern = $test_name_pattern === null
                             ? PHPUnitPatterns::$test_name_pattern
                             : $test_name_pattern;
  }

  private function setTestCommand(bool $redirect = true): void {
    if ($this->test_command === null) {
      $this->test_command = get_runtime_build()." ".__DIR__.
                            "/vendor/bin/phpunit --debug";
    } else {
      $this->test_command .= " --debug";
    }
    if ($this->config_file !== null) {
      $this->test_command .= " -c ".$this->config_file;
    }
    if ($this->parallel) {
      if (Options::$test_by_single_test) {
        $this->test_command .= " --filter";
      }
      $this->test_command .= " '%test%'";
    }
    if ($redirect) {
      $this->test_command .= " 2>&1";
    }

    verbose("General test command for: ".$this->name." is: ".
            $this->test_command . "\n", Options::$verbose);
  }

  private function setTestFilePattern(?string $test_file_pattern = null):
                                        void {
    $this->test_file_pattern = $test_file_pattern === null
                             ? PHPUnitPatterns::$test_file_pattern
                             : $test_file_pattern;
  }

  private function setConfigFile(?string $config_file = null): void {
    if ($config_file == null) {
      // 2 possibilities, phpunit.xml and phpunit.xml.dist for configuration
      $phpunit_config_files = Set {'phpunit.xml', 'phpunit.xml.dist'};
      $this->config_file = find_first_file_recursive($phpunit_config_files,
                                                     $this->test_path,
                                                     false);
    } else {
      $this->config_file = Options::$frameworks_root."/".$config_file;
    }

    if ($this->config_file !== null) {
      verbose("Using phpunit xml file in: ".$this->config_file."\n",
              Options::$verbose);
      // For now, remove any logging and code coverage settings from
      // the configuration file.
      $config_data = simplexml_load_file($this->config_file);
      if ($config_data->logging !== null) {
        unset($config_data->logging);
      }
      file_put_contents($this->config_file, $config_data->saveXML());
    } else {
      verbose("No phpunit xml file found for: ".$this->name.".\n",
              Options::$verbose);
    }
  }

  //********************
  // Private getters
  //********************
  private function getBlacklist(): ?Set {
    return $this->blacklist;
  }

  private function getClownylist(): ?Set {
    return $this->clownylist;
  }

  private function getPullRequests(): ?array<int, array> {
    return $this->pull_requests;
  }

  private function getConfigFile(): ?string {
    return $this->config_file;
  }

  private function getTestFilePattern(): string {
    return $this->test_file_pattern;
  }

  //********************
  // Public functions
  //********************

  // We may have to special case frameworks that don't use
  // phpunit for their testing (e.g. ThinkUp)
  public function getPassPercentage(): mixed {
    if (filesize($this->stats_file) === 0) {
      verbose("Stats File: ".$this->stats_file." has no content. Returning ".
              "fatal\n", Options::$verbose);
      return Statuses::FATAL;
    }

    $num_tests = 0;
    $num_errors_failures = 0;

    // clean pattern represents: OK (364 tests, 590 assertions)
    // error pattern represents: Tests: 364, Assertions: 585, Errors: 5.
    $match = array();
    $handle = fopen($this->stats_file, "r");
    if ($handle) {
      while (($line = fgets($handle)) !== false) {
        $line = rtrim($line, PHP_EOL);
        if (preg_match(PHPUnitPatterns::$tests_ok_pattern,
                       $line, $match) === 1) {
          // We have ths pattern: OK (364 tests, 590 assertions)
          // We want the first match of digits
          preg_match("/[0-9]+(?= )/", $line, $match);
          $num_tests += (int) $match[0];
        } else if (preg_match(PHPUnitPatterns::$tests_failure_pattern,
                       $line, $match) === 1) {
          // We have this pattern: Tests: 364, Assertions: 585, Errors: 5.
          // Break out each type into an array
          $results_arr = str_getcsv($match[0]);
          // Start with a default map of values for the pattern to make
          // the parsing for the math simpler.
          $parsed_results = Map {"Tests" => 0, "Errors" => 0, "Failures" => 0,
                                 "Skipped" => 0, "Incomplete" => 0 };
          foreach ($results_arr as $result) {
            // Strip spaces, then look for the : separator
            $res_arr = split(":", str_replace(" ", "", $result));
            // Remove any possible periods.
            $parsed_results[$res_arr[0]] =
                          (int)(str_replace(".", "", $res_arr[1]));
          }
          // Removed skipped and incomplete tests
          $num_tests +=
            (float)($parsed_results["Tests"] - $parsed_results["Skipped"] -
            $parsed_results["Incomplete"]);
          $num_errors_failures +=
            (float)($parsed_results["Errors"] + $parsed_results["Failures"]);
        } else if ($line === Statuses::FATAL || $line === Statuses::UNKNOWN ||
                   $line === Statuses::TIMEOUT) {
          $num_tests += 1;
          $num_errors_failures += 1;
        } else if ($line === Statuses::SKIP) {
          // If status is SKIP, then we just move on and don't count either way.
        } else if ($this->individual_tests->contains($line) ||
                   $this->test_files->contains($line)) {
          // Just skip over the test names or test file. They are in the stats
          // file as context for the numbers
        } else if ($line === $this->name) {
          // For frameworks running in serial, just the framework name will
          // be printed right now. i.e., Runner::$name will be the framework
          // name. Like an actual test name, do nothing. See Pear:
          //
          // pear
          // Tests: 678, Assertions: 678, Failures: 29, Skipped: 24.
        }
        else {
          error_and_exit("The stats file for ".$this->name." is corrupt! It ".
                         "should only have test names and statuses in it.\n",
                         Options::$csv_only);
        }
      }
      // Count blacklisted tests as failures
      // Note clownylisted tests do not count in the stats (they are essentially
      // no-ops)
      if($this->blacklist !== null) {
        foreach ($this->blacklist as $file) {
          $c = $this->countIndividualTests($file);
          $num_tests += $c;
          $num_errors_failures += $c;
        }
      }
    } else {
      // If we cannot open the stats file, return Fatal
      $pct = Statuses::FATAL;
    }
    if ($num_tests > 0) {
      $pct = round(($num_tests - $num_errors_failures) / $num_tests, 4) * 100;
    } else {
      $pct = Statuses::FATAL;
    }

    verbose(strtoupper($this->name).
            " TEST COMPLETE with pass percentage of: ".$pct."\n",
            Options::$verbose);
    verbose("Stats File: ".$this->stats_file."\n", Options::$verbose);

    return $pct;
  }

  public function prepareCurrentTestStatuses(string $status_code_pattern,
                                           string $stop_parsing_pattern): void {
    $file = fopen($this->expect_file, "r");

    $matches = array();
    $line = null;
    $tests = Map {};

    while (!feof($file)) {
      $line = fgets($file);
      if (preg_match($stop_parsing_pattern, $line, $matches) === 1) {
        break;
      }
      if (preg_match($this->test_name_pattern, $line, $matches) === 1) {
        // Get the next line for the expected status for that test
        $status = rtrim(fgets($file), PHP_EOL);
        $tests[$matches[0]] = $status;
      }
    }
    if ($tests->isEmpty()) {
      $this->current_test_statuses = null;
    } else {
      $this->current_test_statuses = $tests;
    }

  }

  public function clean(): void {
    // Get rid of any old data, except the expect file and test info, of course.
    unlink($this->out_file);
    unlink($this->diff_file);
    unlink($this->errors_file);
    unlink($this->stats_file);
    unlink($this->fatals_file);

    if (Options::$generate_new_expect_file) {
      unlink($this->expect_file);
      verbose("Resetting the expect file for ".$this->name.". ".
              "Establishing new baseline with gray dots...\n",
              !Options::$csv_only);
    }
  }

  public static function sortFile(string $file): bool {
    $results = Map {};
    $handle = fopen($file, "r");
    if ($handle) {
      while (!feof($handle)) {
        // trim out newline since Map doesn't like them in its keys
        $test = rtrim(fgets($handle), PHP_EOL);
        if ($test !== "") {
          $status = rtrim(fgets($handle), PHP_EOL);
          $results[$test] = $status;
        }
      }
      if (!ksort($results)) { return false; }
      fclose($handle);
      $contents = "";
      foreach ($results as $test => $status) {
        $contents .= $test.PHP_EOL;
        $contents .= $status.PHP_EOL;
      }
      if (file_put_contents($file, $contents) === false) { return false; }
      return true;
    }
    return false;
  }

  //********************
  // Protected functions
  //********************

  // This function should only be called once for framework (assuming you don't
  // delete the framework from your repo). The proxy could make things a bit
  // adventurous, so we will see how this works out after some time to test it
  // out
  protected function install(): void {
    verbose("Installing ".$this->name.
            ". You will see white dots during install.....\n",
            !Options::$csv_only);
    // Get rid of any test file and test information. Why? Well, for example,
    // just in case the frameworks are being installed in a different directory
    // path. The tests files file would have the wrong path information then.
    unlink($this->tests_file);
    unlink($this->test_files_file);
    $this->installCode();
    $this->installDependencies();
    if ($this->pull_requests != null) {
      $this->installPullRequests();
    }
    $this->disableTestFiles();
  }

  protected function isInstalled(): bool {
    /*****************************************
     *  See if framework is already installed.
     *****************************************/
    if (!(file_exists($this->install_root))) {
      return false;
    }

    // Get current branch/hash information
    $git_head_file =$this->install_root."/.git/HEAD";
    $git_head_info = trim(file_get_contents($git_head_file));

    // The commit hash has changed and we need to download new code
    if ($git_head_info !== $this->git_commit) {
      verbose("Redownloading ".$this->name." because git commit changed...\n",
              !Options::$csv_only);
      remove_dir_recursive($this->install_root);
      return false;
    }

    if (Options::$force_redownload) {
      verbose("Forced redownloading of ".$this->name."...\n",
              !Options::$csv_only);
      remove_dir_recursive($this->install_root);
      return false;
    }

    if (Options::$get_latest_framework_code) {
      verbose("Get latest code for ".$this->name."...\n",
              !Options::$csv_only);
      remove_dir_recursive($this->install_root);
      return false;
    }

    verbose($this->name." already installed.\n", Options::$verbose);
    return true;
  }

  //********************
  // Private functions
  //********************
  private function installCode(): void {
     // Get the source from GitHub
    verbose("Retrieving framework ".$this->name."....\n", Options::$verbose);
    $git_command = "git clone";
    $git_command .= " ".$this->git_path;
    $git_command .= " -b ".$this->git_branch;
    $git_command .= " ".$this->install_root;

    // "frameworks" directory will be created automatically on first git clone
    // of a framework.

    $git_ret = run_install($git_command, __DIR__, ProxyInformation::$proxies);
    if ($git_ret !== 0) {
      error_and_exit("Could not download framework ".$this->name."!\n",
                     Options::$csv_only);
    }

    // If we are using --latest or --latest-record, we checkout from a branch
    // above, then get an updated commit hash to put in our frameworks.json file
    if (Options::$get_latest_framework_code) {
      $this->git_commit = trim(file_get_contents($this->install_root.
                                                 "/.git/refs/heads/".
                                                 $this->git_branch));
      Options::$framework_info[$this->name]['commit'] = $this->git_commit;
    }

    // Checkout out our baseline test code via SHA or branch
    $git_command = "git checkout";
    $git_command .= " ".$this->git_commit;
    $git_ret = run_install($git_command, $this->install_root,
                           ProxyInformation::$proxies);
    if ($git_ret !== 0) {
      remove_dir_recursive($this->install_root);
      error_and_exit("Could not checkout baseline code for ". $this->name.
                     "! Removing framework!\n", Options::$csv_only);
    }
  }

  private function prepareOutputFiles(): void {
    if (!(file_exists(Options::$results_root))) {
      mkdir($path, 0755, true);
    }
    $this->out_file = Options::$results_root."/".$this->name.".out";
    $this->expect_file = Options::$results_root."/".$this->name.".expect";
    $this->diff_file = Options::$results_root."/".$this->name.".diff";
    $this->errors_file = Options::$results_root."/".$this->name.".errors";
    $this->fatals_file = Options::$results_root."/".$this->name.".fatals";
    $this->stats_file = Options::$results_root."/".$this->name.".stats";
    $this->tests_file = Options::$results_root."/".$this->name.".tests";
    $this->test_files_file = Options::$results_root."/".$this->name.
                             ".testfiles";
  }

  private function findTests(): void {
    $first_time = false;
    if (!file_exists($this->tests_file) ||
        !file_exists($this->test_files_file)) {
      $first_time = true;
      $find_tests_command = get_runtime_build()." TestFinder.php ";
      $find_tests_command .= " --framework-name ".$this->name;
      $find_tests_command .= " --tests-file ".$this->tests_file;
      $find_tests_command .= " --test-files-file ".$this->test_files_file;
      $find_tests_command .= " --test-path ".$this->test_path;
      $find_tests_command .= " --test-file-pattern \"".$this->test_file_pattern.
                             "\"";
      $find_tests_command .= " --config-file ".$this->config_file;
      $find_tests_command .= " --test-find-mode ".$this->test_find_mode;
      if ($this->bootstrap_file !== null) {
        $find_tests_command .= " --bootstrap-file ".$this->bootstrap_file;
      };
      $descriptorspec = array(
        0 => array("pipe", "r"),
        1 => array("pipe", "w"),
        2 => array("pipe", "w"),
      );
      $pipes = null;
      verbose("Command used to find the test files and tests for ".$this->name.
              ": ".$find_tests_command."\n", Options::$verbose);
      $proc = proc_open($find_tests_command, $descriptorspec, $pipes, __DIR__);
      if (is_resource($proc)) {
        $pid = proc_get_status($proc)["pid"];
        pcntl_waitpid($pid, $child_status);
        fclose($pipes[0]);
        fclose($pipes[1]);
        fclose($pipes[2]);
        proc_close($proc);
        if (!pcntl_wifexited($child_status) ||
            pcntl_wexitstatus($child_status) !== 0) {
          unlink($this->tests_file);
          unlink($this->test_files_file);
          error_and_exit("Could not get tests for ".$this->name,
                         Options::$csv_only);
        }
      } else {
        error_and_exit("Could not open process tp get tests for ".$this->name,
                       Options::$csv_only);
      }
    }

    $this->individual_tests = Set {};
    $this->individual_tests->addAll(file($this->tests_file,
                                         FILE_IGNORE_NEW_LINES));
    $this->test_files = Set {};
    $this->test_files->addAll(file($this->test_files_file,
                                   FILE_IGNORE_NEW_LINES));
    if ($first_time) {
      verbose("Found ".count($this->individual_tests)." tests for ".$this->name.
              ". Each test could have more than one data set, making the ".
              "total number be actually higher.\n", !Options::$csv_only);
    }
  }

  private function disableTestFiles(): void {
    $this->blacklist = $this->disable($this->blacklist,
                                      ".disabled.hhvm.blacklist");
    $this->clownylist = $this->disable($this->clownylist,
                                     ".disabled.hhvm.clownylist");
    verbose(count($this->blacklist)." files were blacklisted (auto fail) ".
            $this->name."...\n", Options::$verbose);
    verbose(count($this->clownylist)." files were clownylisted (no-op/no run) ".
            $this->name."...\n", Options::$verbose);
  }

  private function disable(?Set $tests, string $suffix): ?Set {
    if ($tests === null) { return null; }
    $updated_tests = Set {};
    foreach ($tests as $t) {
      // Check if we are already disabled first
      if (!file_exists($t.$suffix)) {
        if (!rename($t, $t.$suffix)) {
          error_and_exit("Could not disable ".$t. " in ".$this->name."!");
        }
      }
      $updated_tests->add($t.$suffix);
    }
    return $updated_tests;
  }

  private function installDependencies(): void {
    $composer_json_path = find_first_file_recursive(Set {"composer.json"},
                                                  $this->install_root, true);
    verbose("composer.json found in: $composer_json_path\n", Options::$verbose);
    // Check to see if composer dependencies are necessary to run the test
    if ($composer_json_path !== null) {
      verbose("Retrieving dependencies for framework ".$this->name.".....\n",
              Options::$verbose);
      // Use the timeout to avoid curl SlowTimer timeouts and problems
      $dependencies_install_cmd = get_runtime_build();
      // Only put this timeout if we are using hhvm
      if (Options::$zend_path === null) {
        $dependencies_install_cmd .= " -v ResourceLimit.SocketDefaultTimeout".
                                     "=30";
      }
      $dependencies_install_cmd .= " ".__DIR__."/composer.phar install --dev";
      $install_ret = run_install($dependencies_install_cmd, $composer_json_path,
                                 ProxyInformation::$proxies);

      if ($install_ret !== 0) {
        // Let's just really make sure the dependencies didn't get installed
        // by checking the vendor directories to see if they are empty.
        $fw_vendor_dir = find_first_file_recursive(Set {"vendor"},
                                                 $this->install_root,
                                                 false);
        if ($fw_vendor_dir !== null) {
          // If there is no content in the directories under vendor, then we
          // did not get the dependencies.
          if (any_dir_empty_one_level($fw_vendor_dir)) {
            remove_dir_recursive($this->install_root);
            error_and_exit("Couldn't download dependencies for ".$this->name.
                           ". Removing framework. You can try the --zend ".
                           "option.\n", Options::$csv_only);
          }
        } else { // No vendor directory. Dependencies could not have been gotten
          remove_dir_recursive($this->install_root);
          error_and_exit("Couldn't download dependencies for ".$this->name.
                         ". Removing framework. You can try the --zend ".
                         "option.\n", Options::$csv_only);
        }
      }
    }
  }

  private function installPullRequests(): void {
    verbose("Merging some upstream pull requests for ".$this->name."\n",
            Options::$verbose);
    foreach ($this->pull_requests as $pr) {
      $dir = $pr["pull_dir"];
      $rep = $pr["pull_repo"];
      $gc = $pr["git_commit"];
      $type = $pr["type"];
      $move_from_dir = null;
      $dir_to_move = null;
      chdir($dir);
      $git_command = "";
      verbose("Pulling code from ".$rep. " and branch/commit ".$gc."\n",
              Options::$verbose);
      if ($type === "pull") {
        $git_command = "git pull --no-rebase ".$rep." ".$gc;
      } else if ($type === "submodulemove") {
        $git_command = "git submodule add -b ".$gc." ".$rep;
        $move_from_dir = $pr["move_from_dir"];
        $dir_to_move = $pr["dir_to_move"];
      }
      verbose("Pull request command: ".$git_command."\n", Options::$verbose);
      $git_ret = run_install($git_command, $dir,
                             ProxyInformation::$proxies);
      if ($git_ret !== 0) {
        remove_dir_recursive($this->install_root);
        error_and_exit("Could not get pull request code for ".$this->name."!".
                       " Removing framework!\n", Options::$csv_only);
      }
      if ($dir_to_move !== null) {
        $mv_command = "mv ".$dir_to_move." ".$dir;
        verbose("Move command: ".$mv_command."\n", Options::$verbose);
        exec($mv_command);
        verbose("After move, removing: ".$move_from_dir."\n",
                Options::$verbose);
        remove_dir_recursive($move_from_dir);
      }
      chdir(__DIR__);
    }
  }

  // Right now this is just an estimate since one test can
  // have a bunch of different data sets sent to it via
  // a data provider, making one test really into n tests.
  private function countIndividualTests(string $testfile): int {
    if (strpos($testfile, ".phpt") !== false) {
      return 1;
    }
    $contents = file_get_contents($testfile);
    $matches = null;
    return preg_match_all(PHPUnitPatterns::$test_method_name_pattern,
                          $contents, $matches);
  }
}
