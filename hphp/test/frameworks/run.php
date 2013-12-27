#!/usr/bin/env php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

/*
 * This script allows us to more easily test the key OSS PHP frameworks
 * that is helping bring HHVM closer to parity.
 *
 * Key features:
 *
 *   - Autodownload of frameworks, so we don't have to add 3 GB of frameworks to
 *     our official repo. This can be a bit flakey due to our proxy; so we
 *     we will see how this works out moving forward. If the framework does not
 *     exist in your local repo, it gets downloaded to your dev box. The
 *     .gitignore will ensure that the frameworks aren't added to the official
 *     repo.
 *
 *   - Run a single test suite, all of the test suites or a custom set of
 *     multiple test suites. Currently, all of the available tests live in the
 *     code itself. See the help for the syntax on how to run the tests in each
 *     available mode.
 *
 *   - Multiple test suites are run in separate processes, making the entire
 *     testing process a bit faster. This also helps us prepare for
 *     incorporating this with our official test suite.
 *
 *   - The creation (and appending to) a summary file that lists all frameworks
 *     run and the pass percentage (or fatal) of each framework. The impetus for
 *     this file is the "OSS Parity" snapshot on our team TV screen.
 *
 *   - Raw statistics for each test suite are put in a raw results file for
 *     examination.
 *
 *   - Diff files showing the tests names and results that are different than
 *     expected for the test suite.
 *
 *   - Error files showing all the errors and failures from running the test,
 *     suite or the fatal if the framework fatals.
 *
 *   - Timeout option for running individual tests. There is a default of 60
 *     seconds to run each test, but this can be shortened or lengthened as
 *     desired.
 *
 *   - Enhanced data output by the script to include "diff" type information
 *     about why a passing percentage is different from a previous run,
 *     particularly from a regression perspective. For example, what tests
 *     caused the regression.
 *
 * Comment about the frameworks:
 *
 *   - Have a 'git_commit' field to ensure consistency across test runs as we
 *     may have different download times for people, as well as redownloads.
 *     The latest SHA at the time was used for the value.
 *
 *   - In order to get frameworks to work correctly, may need to grab more code
 *     via some sort of pull request:
 *
 *     - pull:      The code we need is in a dir that doesn't affect the
 *                  primary branch or SHA (e.g., 'vendor') and we can just do
 *                  a 'git pull' since any branch or HEAD change doesn't matter
 *     - submodule: The code we are adding may be in the root framework dir
 *                  so that can affect the framework branch or SHA. If we
 *                  pull/merge, the HEAD SHA changes. (FIX IF THIS DOESN'T
 *                  HAVE TO BE THE CASE). And, if that happens, we will always
 *                  be redownloading the framework since the SHA is different
 *                  than what we expect. Use a submodule/move technique.
 *
 *   - Clowny tests that fail both Zend and HHVM (#WTFPear). We treat these
 *     tests as no-ops with respect to calculation.
 *
 *   - Blacklist tests that are causing problems with this script running. E.g,
 *     deadlocks
 *
 * Future enhancements:
 *
 *   - Integreate the testing with our current "test/run" style infrastructure
 *     for official pass/fail statistics when we have diffs.
 *
 *   - Special case frameworks that don't use PHPUnit (e.g. Thinkup).
 *
 *   - Decide whether proxy-based code needs to be solidified better or whether
 *     we should just add the frameworks to our repo officially.
 *
 *
*/
$FBCODE_ROOT = __DIR__.'/../../..';

require_once $FBCODE_ROOT.'/hphp/tools/command_line_lib.php';
require_once __DIR__.'/SortedIterator.php';
require_once __DIR__.'/utils.php';
require_once __DIR__.'/TestFindModes.php';

type Color = string;
class Colors {
  const Color GREEN = "\033[1;32m";
  const Color RED = "\033[0;31m";
  const Color BLUE = "\033[0;34m";
  const Color GRAY = "\033[1;30m";
  const Color LIGHTBLUE = "\033[1;34m";
  const Color YELLOW = "\033[0;33m";
  const Color NONE = "\033[0m";
}

type Status = string;
class Statuses {
  const Status FATAL = "FATAL";
  const Status UNKNOWN = "UNKNOWN STATUS";
  const Status PASS = ".";
  const Status FAIL = "F";
  const Status ERROR = "E";
  const Status INCOMPLETE = "I";
  const Status SKIP = "S";
  const Status TIMEOUT = "TIMEOUT";
  const Status BLACKLIST = "BLACKLIST";
  const Status CLOWNY = "CLOWNY";
  const Status WARNING = "WARNING";
}

// Put the proxy information in its own "struct-like" class for easy access
// in case folks outside this proxy wall need to change them.
class ProxyInformation {

  public static Map $proxies = null;

  // Determine through a poor man's method whether a proxy will be required to
  // get out to the internet. If we get headers back, then a proxy is not
  // required. If we get false back, then a header is required.
  public static function is_proxy_required(
                             string $test_url = 'http://www.google.com'): bool {
    if (strpos(gethostname(), "facebook.com") || !(get_headers($test_url))) {
      self::$proxies =
        Map {
          "HOME" => getenv("HOME"),
          "http_proxy" => "http://fwdproxy.any.facebook.com:8080",
          "https_proxy" => "http://fwdproxy.any.facebook.com:8080",
          "HTTPS_PROXY" => "http://fwdproxy.any.facebook.com:8080",
          "HTTP_PROXY" => "http://fwdproxy.any.facebook.com:8080",
          "HTTP_PROXY_REQUEST_FULLURI" => "true",
          "no_proxy" => "facebook.com,fbcdn.net",
          "NO_PROXY" => "facebook.com,fbcdn.net",
        };
        return true;
    } else {
        self::$proxies = Map { "HOME" => getenv("HOME") };
        return false;
    }
  }
}

class PHPUnitPatterns {
  // Matches:
  //   PrettyExceptionsTest::testReturnsDiagnostics
  //   Assetic\Test\::testMethods with data set #1 ('getRoot')'
  //   Composer\Test\::testMe with data set "parses dates w/ -"
  // The "with data set" can either have a # or " after it and then any char
  // before a resulting " or (
  // Four \\\\ needed to match one \
  // stackoverflow.com/questions/4025482/cant-escape-the-backslash-with-regex
  static string $test_name_pattern =
  "/[_a-zA-Z0-9\\\\]*::[_a-zA-Z0-9]*( with data set (\".*?\"|#[0-9]+))?/";

  static string $pear_test_name_pattern =
 "/[\-_a-zA-Z0-9\.\/]*\.phpt/";

  // Matches:
  //    E
  //    .
  //    .  252 / 364 ( 69%)
  //    .HipHop Warning
  // That last example happened in Magento
  static string $status_code_pattern =
  "/^[\.SFEI]$|^[\.SFEI](HipHop)|^[\.SFEI][ \t]*[0-9]* \/ [0-9]* \([ 0-9]*%\)/";

  // Don't want to parse any more test names after the Time line in the
  // results. Any test names after that line are probably detailed error
  // information.
  static string $stop_parsing_pattern =
 "/^Time: \d+(\.\d+)? (second[s]?|ms|minute[s]?|hour[s]?), Memory: \d+(\.\d+)/";

  static string $tests_ok_pattern = "/^OK \(\d+ test[s]?, \d+ assertion[s]?\)/";
  static string $tests_failure_pattern = "/^Tests: \d+, Assertions: \d+.*[.]/";

  static string $header_pattern =
                "/^PHPUnit \d+.[0-9a-zA-Z\-\.]*( by Sebastian Bergmann.)?/";

  static string $config_file_pattern = "/^Configuration read from/";

  static string $xdebug_pattern = "/^The Xdebug extension is not loaded./";

  // Paris and Idiorm have tests with ending digits (e.g. Test53.php)
  static string $test_file_pattern =
                "/.*(\.phpt|Test[\d]*\.php|test[\d]*\.php)$/";
  static string $pear_test_file_pattern = "/.*(\.phpt)$/";
  static string $facebook_sdk_test_file_pattern = "/.*(tests\.php)$/";
  static string $mediawiki_test_file_pattern = "/.*(\Test.*\.php)$/";

  static string $tests_ok_skipped_inc_pattern =
               "/^OK, but incomplete or skipped tests!/";
  static string $num_errors_failures_pattern =
               "/^There (was|were) \d+ (failure|error)[s]?\:/";
  static string $num_skips_inc_pattern =
               "/^There (was|were) \d+ (skipped|incomplete) test[s]?\:/";
  static string $failures_header_pattern = "/^FAILURES!/";
  static string $no_tests_executed_pattern = "/^No tests executed!/";

  static string $hhvm_warning_pattern =
                                     "/^(HipHop|HHVM|hhvm) (Warning|Notice)/";
  static string $hhvm_fatal_pattern =
  "/(^(HipHop|HHVM|hhvm) Fatal)|(^hhvm:)|(^Core dumped: Segmentation fault)/";

  static string $phpunit_exception_with_hhvm_warning =
    "/^PHPUnit_Framework_Exception: (HipHop|HHVM|hhvm) (Warning|Notice)/";
  static string $test_method_name_pattern = "/public function test|\@test/";
}

class Options {
  // seconds to run any individual test for any framework
  public static int $timeout = 90;
  public static bool $verbose = false;
  public static bool $csv_only = false;
  public static bool $csv_header = false;
  public static bool $force_redownload = false;
  public static bool $generate_new_expect_file = false;
  public static string $zend_path = null;
  public static bool $all = false;
  public static bool $allexcept = false;
  public static bool $test_by_single_test = false;
  public static string $results_root;
  public static string $script_errors_file;

  public static function parse(OptionInfoMap $options, array $argv): Vector {
    self::$results_root = __DIR__."/results";
    // Put any script error to a file when we are in a mode like --csv and
    // want to control what gets printed to something like STDOUT.
    self::$script_errors_file = self::$results_root."/_script.errors";
    unlink(self::$script_errors_file);

    // Don't use $argv[0] which just contains the program to run
    $framework_names = Vector::fromArray(array_slice($argv, 1));

    // HACK: Yes, this next bit of "removeKey" code is hacky, maybe even clowny.
    // We can fix the command_line_lib.php to maybe make things a bit better.

    // It is possible that the $framework_names vector has a combiniation
    // of command line options (e.g., verbose and timeout) that should be
    // removed before running the tests. Remeber all these option values are
    // already set in $options. They are just artificats of $argv right now.
    // Although, there is a failsafe when checking if the framework exists that
    // would weed command line opts out too.


    // Can't run all the framework tests and "all but" at the same time
    if ($options->containsKey('all') && $options->containsKey('allexcept')) {
      error_and_exit("Cannot use --all and --allexcept together");
    } else if ($options->containsKey('all')) {
      self::$all = true;
      $framework_names->removeKey(0);
    } else if ($options->containsKey('allexcept')) {
      self::$allexcept = true;
      $framework_names->removeKey(0);
    }

    // Can't be both summary and verbose.
    if ($options->containsKey('csv') && $options->containsKey('verbose')) {
      error_and_exit("Cannot be --csv and --verbose together");
    }
    else if ($options->containsKey('csv')) {
      self::$csv_only = true;
      // $tests[0] may not even be "summary", but it doesn't matter, we are
      // just trying to make the count right for $frameworks
      $framework_names->removeKey(0);
    }
    else if ($options->containsKey('verbose')) {
      self::$verbose = true;
      $framework_names->removeKey(0);
    }

    if ($options->contains('csvheader')) {
      self::$csv_header = true;
      $framework_names->removeKey(0);
    }

    // Can't run framework tests both by file and single test
    if ($options->containsKey('by-file') &&
        $options->containsKey('by-single-test')) {
      error_and_exit("Cannot specify both by-file or by-single-test");
    } else if ($options->contains('by-single-test')) {
      self::$test_by_single_test = true;
      $framework_names->removeKey(0);
    } else if ($options->contains('by-file')) {
      // Nothing to set here since this is the default, but remove the key
      $framework_names->removeKey(0);
    }

    verbose("Script running...Be patient as some frameworks take a while with ".
            "a debug build of HHVM\n", self::$verbose);

    if (ProxyInformation::is_proxy_required()) {
      verbose("Looks like proxy may be required. Setting to default FB proxy ".
           "values. Please change Map in ProxyInformation to correct values, ".
           "if necessary.\n", self::$verbose);
    }

    if ($options->containsKey('timeout')) {
      self::$timeout = (int) $options['timeout'];
      // Remove timeout option and its value from the $framework_names vector
      $framework_names->removeKey(0);
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('zend')) {
      verbose ("Will try Zend if necessary. If Zend doesn't work, the script ".
           "will still continue; the particular framework on which Zend ".
           "was attempted may not be available though.\n", self::$verbose);
      self::$zend_path = $options['zend'];
      $framework_names->removeKey(0);
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('redownload')) {
      self::$force_redownload = true;
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('record')) {
      self::$generate_new_expect_file = true;
      $framework_names->removeKey(0);
    }

    // This will return just the name of the frameworks passed in, if any left
    // (e.g. --all may have been passed, in which case the Vector will be
    // empty)
    return $framework_names;
  }
}

abstract class Framework {
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
  private Map $env_vars;

  private string $install_root;
  private string $git_path;
  private string $git_commit;
  private Set $blacklist;
  private Set $clownylist;
  private Vector $pull_requests;
  private Map $args_for_tests;
  private string $test_command;
  private Set $individual_tests = null;
  private string $bootstrap_file = null;
  private string $config_file = null;

  // $name, $parallel and $test_fine_mode are constructor promoted
  // Assume the framework unit tests will be run in parallel until otherwise
  // proven. Also assume that tests will be found by reflecting over the
  // framework. However, some require that we use php tokens or are found via
  // phpt files.
  protected function __construct(private string $name,
                                 private bool $parallel = true,
                                 private string $test_find_mode =
                                                TestFindModes::REFLECTION) {

    // Get framework information and set all needed properties. Beyond
    // the install root, git info and test search roots, the other
    // properties are optional and may or may not be set
    $info = $this->getInfo();
    if (!$info->containsKey("install_root") ||
        !$info->containsKey("git_path") ||
        !$info->containsKey("git_commit") ||
        !$info->containsKey("test_path")) {
      throw new Exception("Provide install, git and test file search info");
    }

    // Set Framework information for install. These are the five necessary
    // properties for a proper install, with pull_requests being optional.
    $this->setInstallRoot($info->get("install_root"));
    $this->setGitPath($info->get("git_path"));
    $this->setGitCommit($info->get("git_commit"));
    $this->setPullRequests($info->get("pull_requests"));
    $this->setBlacklist($info->get("blacklist"));
    $this->setClownylist($info->get("clownylist"));
    $this->setTestNamePattern($info->get("test_name_pattern"));
    $this->setTestFilePattern($info->get("test_file_pattern"));
    $this->setTestPath($info->get("test_path"));

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
    $this->setEnvVars($info->get("env_vars"));
    $this->setConfigFile($info->get("config_file"));
    $this->setBootstrapFile($info->get("bootstrap"));
    $this->setTestCommand($info->get("test_command"));
    $this->setArgsForTests($info->get("args_for_tests"));
    $this->prepareOutputFiles();
    $this->findTests();
  }

  abstract protected function getInfo(): Map;

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
    if (Options::$test_by_single_test) {
      return $this->individual_tests;
    } else {
      return $this->test_files;
    }
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

  //********************
  // Private setters
  //********************
  private function setGitPath(string $git_path): void {
    $this->git_path = $git_path;
  }

  private function setGitCommit(string $git_commit): void {
    $this->git_commit = $git_commit;
  }

  private function setBlacklist(?Set $blacklist): void {
    $this->blacklist = $blacklist;
  }

  private function setClownylist(?Set $clownylist): void {
    $this->clownylist = $clownylist;
  }

  private function setPullRequests(?Vector $pull_requests): void {
    $this->pull_requests = $pull_requests;
  }

  private function setBootstrapFile(?string $bootstrap_file = null): void {
    $this->bootstrap_file = $bootstrap_file;
  }

  private function setTestPath(string $test_path): void {
    $this->test_path = $test_path;
  }

  private function setInstallRoot(string $install_root): void {
    $this->install_root = $install_root;
  }

  private function setEnvVars(?Map $env_vars): void {
    $this->env_vars = $env_vars;
  }

  private function setArgsForTests(?Map $args_for_tests): void {
    $this->args_for_tests = $args_for_tests;
  }

  private function setTestNamePattern(?string $test_name_pattern = null):
                                        void {
    // Test name pattern can be different depending on the framework,
    // although most follow the default.
    $this->test_name_pattern = $test_name_pattern === null
                             ? PHPUnitPatterns::$test_name_pattern
                             : $test_name_pattern;
  }

  private function setTestCommand(?string $test_command = null,
                                    bool $redirect = true): void {
    if ($test_command === null) {
      $this->test_command = get_runtime_build()." ".__DIR__.
                            "/vendor/bin/phpunit --debug";
    } else {
      $this->test_command = $test_command." --debug";
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

      if ($this->config_file !== null) {
        verbose("Using phpunit xml file in: ".$this->config_file."\n",
                Options::$verbose);
      } else {
        verbose("No phpunit xml file found for: ".$this->name.".\n",
                Options::$verbose);
      }
    } else {
      $this->config_file = $config_file;
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

  private function getPullRequests(): ?Vector {
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
    // Get rid of any old data, except the expect file, of course.
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
    $results = StableMap {};
    $handle = fopen($file, "r");
    if ($handle) {
      while (!feof($handle)) {
        // trim out newline since StableMap doesn't like them in its keys
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

    $this->installCode();
    $this->installDependencies();
    if ($this->pull_requests != null) {
      $this->installPullRequests();
    }
    $this->disableTestFiles();
  }

  protected function isInstalled(): bool {
    /****************************************
     *  See if framework is already installed
     *  installed.
     ***************************************/
    $git_head_file =$this->install_root."/.git/HEAD";
    if (!(file_exists($this->install_root))) {
      return false;
    } else if (Options::$force_redownload) {
      verbose("Forced redownloading of ".$this->name."...\n",
              !Options::$csv_only);
      remove_dir_recursive($this->install_root);
      return false;
    // The commit hash has changed and we need to redownload
    } else if (trim(file_get_contents($git_head_file)) !==
               $this->git_commit) {
      verbose("Redownloading ".$this->name." because git commit changed...\n",
              !Options::$csv_only);
      remove_dir_recursive($this->install_root);
      return false;
    }
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
    $git_command .= " ".$this->install_root;

    // "frameworks" directory will be created automatically on first git clone
    // of a framework.
    $git_ret = run_install($git_command, __DIR__, ProxyInformation::$proxies);
    if ($git_ret !== 0) {
      error_and_exit("Could not download framework ".$this->name."!\n",
                     Options::$csv_only);
    }
    // Checkout out our baseline test code via SHA
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
        $find_tests_command .= " --bfile ".$this->bootstrap_file;
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
      $rep = $pr["pull_repository"];
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

class Assetic extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/assetic",
      "git_path" => "https://github.com/ptarjan/assetic.git",
      'git_commit' => "0aada83090e28eff7a6a112f4e2d1a583f017242",
      "test_path" => __DIR__."/frameworks/assetic",
    };
  }
}


class Monolog extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/monolog",
      "git_path" => "https://github.com/Seldaek/monolog.git",
      'git_commit' => "6225b22de9dcf36546be3a0b2fa8e3d986153f57", // stable 1.7
      "test_path" => __DIR__."/frameworks/monolog",
    };
  }
}

class ReactPHP extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/reactphp",
      "git_path" => "https://github.com/reactphp/react.git",
      # current stable - v0.3.3
      'git_commit' => "210c11a6041cfa2ce1701a4870b69475d9081265",
      "test_path" => __DIR__."/frameworks/reactphp",
    };
  }
}

class Ratchet extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/ratchet",
      "git_path" => "https://github.com/cboden/Ratchet.git",
      # current stable - v0.3.0
      'git_commit' => "d756e0b507a5f3cdbf8c59dbb7baf68574dc7d58",
      "test_path" => __DIR__."/frameworks/ratchet",
    };
  }
}

class CodeIgniter extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/CodeIgniter",
      "git_path" => "https://github.com/EllisLab/CodeIgniter.git",
      "git_commit" => "b6fbcbefc8e6e883773f8f5d447413c367da9aaa",
      "test_path" => __DIR__."/frameworks/CodeIgniter/tests",
    };
  }
}

class Composer extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/composer",
      "git_path" => "https://github.com/composer/composer.git",
      "git_commit" => "7defc95e4b9eded1156386b269a9d7d28fa73710",
      "test_path" => __DIR__."/frameworks/composer",
    };
  }
}

class Doctrine2 extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/doctrine2",
      "git_path" => "https://github.com/doctrine/doctrine2.git",
      "git_commit" => "75d7ac2783345803da1cc211735382f7a4c5d055",
      "test_path" => __DIR__."/frameworks/doctrine2",
      "pull_requests" => Vector {
        Map {
          'pull_dir' => __DIR__."/frameworks/doctrine2/vendor/doctrine/dbal",
          'pull_repository' => "https://github.com/javer/dbal",
          'git_commit' => "hhvm-pdo-implement-interfaces",
          'type' => 'pull',
        },
      },
    };
  }
}

class Drupal extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/drupal",
      "git_path" => "https://github.com/drupal/drupal.git",
      "git_commit" => "adaf8355074ba3e142f61e10f1790382db5defb9",
      "test_path" => __DIR__."/frameworks/drupal/core",
      "clownylist" => Set {
        __DIR__."/frameworks/drupal/core/modules/views/tests/".
        "Drupal/views/Tests/ViewsDataHelperTest.php",
      },
    };
  }
}

class FacebookPhpSdk extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/facebook-php-sdk",
      "git_path" => "https://github.com/facebook/facebook-php-sdk.git",
      "git_commit" => "16d696c138b82003177d0b4841a3e4652442e5b1",
      "test_path" => __DIR__."/frameworks/facebook-php-sdk",
      "test_file_pattern" => PHPUnitPatterns::$facebook_sdk_test_file_pattern,
      "test_command" => get_runtime_build()." ".__DIR__.
                      "/vendor/bin/phpunit --bootstrap tests/bootstrap.php",
    };
  }

  protected function install(): void {
    parent::install();
    verbose("Creating a phpunit.xml for running the pear tests.\n",
            Options::$verbose);
    $phpunit_xml = <<<XML
<phpunit bootstrap="./tests/bootstrap.php">
<testsuites>
  <testsuite name="FacebookPhpSdk">
    <directory suffix="tests.php">tests</directory>
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

class Idiorm extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/idiorm",
      "git_path" => "https://github.com/j4mie/idiorm.git",
      "git_commit" => "3be516b440734811b58bb9d0b458a4109b49af71",
      "test_path" => __DIR__."/frameworks/idiorm",
    };
  }
}

class Joomla extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      'install_root' => __DIR__.'/frameworks/joomla-framework',
      // 'git_path' => 'https://github.com/joomla/joomla-framework.git',
      'git_path' => 'https://github.com/elgenie/joomla-framework',
      'git_commit' => 'f87645575d9f6be213bf34fda3ec5bcf0eeef7a0',
      'test_path' => __DIR__.'/frameworks/joomla-framework',
      "clownylist" => Set {
        // These are subtests which need their own composer set and aren't run
        // by their travis setup
        __DIR__."/frameworks/joomla-framework/".
          "src/Joomla/Google/Tests/JGoogleAuthOauth2Test.php",
        __DIR__."/frameworks/joomla-framework/".
          "src/Joomla/Google/Tests/JGoogleDataAdsenseTest.php",
        __DIR__."/frameworks/joomla-framework/".
          "src/Joomla/Google/Tests/JGoogleDataCalendarTest.php",
        __DIR__."/frameworks/joomla-framework/".
          "src/Joomla/Google/Tests/JGoogleDataPicasaAlbumTest.php",
        __DIR__."/frameworks/joomla-framework/".
          "src/Joomla/Google/Tests/JGoogleDataPicasaPhotoTest.php",
        __DIR__."/frameworks/joomla-framework/".
          "src/Joomla/Google/Tests/JGoogleDataPicasaTest.php",
        __DIR__."/frameworks/joomla-framework/".
          "src/Joomla/Google/Tests/JGoogleDataPlusActivitiesTest.php",
        __DIR__."/frameworks/joomla-framework/".
          "src/Joomla/Google/Tests/JGoogleDataPlusCommentsTest.php",
        __DIR__."/frameworks/joomla-framework/".
          "src/Joomla/Google/Tests/JGoogleDataPlusPeopleTest.php",
        __DIR__."/frameworks/joomla-framework/".
          "src/Joomla/Google/Tests/JGoogleDataPlusTest.php",
        __DIR__."/frameworks/joomla-framework/".
          "src/Joomla/Google/Tests/JGoogleTest.php",
      },
    };
  }
}

class Laravel extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/laravel",
      "git_path" => "https://github.com/laravel/framework.git",
      "git_commit" => "77eab893edd30ccc42722358bee69b1ccea24f6a",
      "test_path" => __DIR__."/frameworks/laravel",
      "args_for_tests" => Map {
        __DIR__."/frameworks/laravel/./tests/Auth/AuthGuardTest.php"
        => "-v JitEnableRenameFunction"
      },
    };
  }
}

class Magento2 extends Framework {
  public function __construct(string $name) {
    parent::__construct($name, true, TestFindModes::TOKEN);
  }

  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/magento2",
      "git_path" => "https://github.com/magento/magento2.git",
      "git_commit" => "a15ecb31976feb4ecb62f85257ff6b606fbdbc00",
      "test_path" => __DIR__."/frameworks/magento2/dev/tests/unit",
    };
  }
}

class Mediawiki extends Framework {
  public function __construct(string $name) {
    parent::__construct($name, true, TestFindModes::TOKEN);
  }

  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/mediawiki-core",
      "git_path" => "https://github.com/ptarjan/mediawiki-core.git",
      "git_commit" => "2a280fcfccfd48fe5bf9479ca02d986367fa33e9",
      "test_path" => __DIR__."/frameworks/mediawiki-core/tests/phpunit",
      "test_file_pattern" => PHPUnitPatterns::$mediawiki_test_file_pattern,
      "config_file" => __DIR__.
                      "/frameworks/mediawiki-core/tests/phpunit/suite.xml",
      "test_command" => get_runtime_build()." phpunit.php ".
                        "--exclude-group=Database,Broken",
      "clownylist" => Set {
        __DIR__."/frameworks/mediawiki-core/tests/phpunit/".
        "includes/HttpTest.php",
        __DIR__."/frameworks/mediawiki-core/tests/phpunit/".
        "includes/libs/CSSJanusTest.php",
      },
    };
  }

  protected function install(): void {
    parent::install();
    verbose("Adding LocalSettings.php file to Mediawiki test dir.\n",
            Options::$verbose);
    $touch_command = "touch ".$this->getInstallRoot()."/LocalSettings.php";
    exec($touch_command);
  }
}


class Paris extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/paris",
      "git_path" => "https://github.com/j4mie/paris.git",
      "git_commit" => "b60d0857d10dec757427b336c427c1f13b6a5e48",
      "test_path" => __DIR__."/frameworks/paris",
    };
  }
}

class Pear extends Framework {
  public function __construct(string $name) {
    // Pear will currently run serially
    parent::__construct($name, false, TestFindModes::PHPT);
  }

  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/pear-core",
      "git_path" => "https://github.com/pear/pear-core.git",
      "git_commit" => "e379594cef09079be131d2fbbb19b1c2256872c2",
      "test_path" => __DIR__."/frameworks/pear-core",
      "test_name_pattern" => PHPUnitPatterns::$pear_test_name_pattern,
      "test_file_pattern" => PHPUnitPatterns::$pear_test_file_pattern,
      "pull_requests" => Vector {
        Map {
          'pull_dir' => __DIR__."/frameworks/pear-core",
          'pull_repository' => "https://github.com/pear/Console_Getopt",
          'git_commit' => "trunk",
          'type' => 'submodulemove',
          'move_from_dir' => __DIR__."/frameworks/pear-core/Console_Getopt",
          'dir_to_move' => __DIR__.
                           "/frameworks/pear-core/Console_Getopt/Console",
        },
        Map {
          'pull_dir' => __DIR__."/frameworks/pear-core",
          'pull_repository' => "https://github.com/pear/XML_Util",
          'git_commit' => "trunk",
          'type' => 'submodulemove',
          'move_from_dir' => __DIR__."/frameworks/pear-core/XML_Util",
          'dir_to_move' => __DIR__."/frameworks/pear-core/XML_Util/XML",
        },
        Map {
          'pull_dir' => __DIR__."/frameworks/pear-core",
          'pull_repository' => "https://github.com/pear/Archive_Tar",
          'git_commit' => "master",
          'type' => 'submodulemove',
          'move_from_dir' => __DIR__."/frameworks/pear-core/Archive_Tar",
          'dir_to_move' => __DIR__."/frameworks/pear-core/Archive_Tar/Archive",
        },
        Map {
          'pull_dir' => __DIR__."/frameworks/pear-core",
          'pull_repository' => "https://github.com/pear/Structures_Graph",
          'git_commit' => "trunk",
          'type' => 'submodulemove',
          'move_from_dir' => __DIR__."/frameworks/pear-core/Structures_Graph",
          'dir_to_move' => __DIR__.
                           "/frameworks/pear-core/Structures_Graph/Structures",
        },
      },
      "clownylist" => Set {
        __DIR__."/frameworks/pear-core/tests/PEAR_Command/".
        "test_registerCommands_standard.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Command_Config/".
        "config-create/test.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Command_Config/".
        "config-create/test_windows.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Command_Config/".
        "config-help/test.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Command_Config/".
        "config-show/test.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Command_Install/".
        "upgrade/test_bug17986.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Command_Package/".
        "convert/test_fail.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Config/".
        "test_getGroupKeys.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Config/".
        "test_getKeys.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader/".
        "test_download_abstractpackage_channelneedsupdating.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader/".
        "test_download_abstractpackage_rest.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader/".
        "test_download_alreadyinstalled.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader/".
        "test_download_complexabstractpackage.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader/".
        "test_download_complexabstractpackage_alphapostfix.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader/".
        "test_download_complexlocalpackage.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader/".
        "test_download_complexlocalpackage_onlyreqdeps.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader/".
        "test_download_complexlocalpackage_optional.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader/".
        "test_download_complexlocaltgz.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader/".
        "test_download_complexremotetgz.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader/".
        "test_upgrade_pear_to_pecl.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader_Package/".
        "test_initialize_abstractpackage_discover.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader_Package/".
        "test_initialize_downloadurl.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader_Package/".
        "test_initialize_invalidabstractpackage5.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader_Package/".
        "test_initialize_invalidabstractpackage6.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader_Package/".
        "test_initialize_invalidabstractpackage_discover.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader_Package/".
        "test_initialize_invaliddownloadurl.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Downloader_Package/".
        "test_mergeDependencies_basic_required_uri.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Installer/".
        "test_install_complexlocalpackage.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Installer/".
        "test_install_complexlocalpackage2.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Installer/".
        "test_install_complexlocalpackage2_force.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Installer/".
        "test_install_complexlocalpackage2_ignore-errors.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Installer/".
        "test_install_complexlocalpackage2_ignore-errorssoft.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Installer/".
        "test_upgrade_complexlocalpackage2.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Installer_Role/".
        "test_getInstallableRoles.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_Installer_Role/".
        "test_getValidRoles.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_PackageFile_v2_Validator/".
        "test_extbinrelease.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_PackageFile_v2_Validator/".
        "test_extsrcrelease.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_PackageFile_v2_Validator/".
        "test_phprelease.phpt",
        __DIR__."/frameworks/pear-core/tests/System/".
        "find_test.phpt",
        __DIR__."/frameworks/pear-core/tests/System/".
        "test_which.phpt",
        __DIR__."/frameworks/pear-core/tests/PEAR_PackageFile_v2_Validator/".
        "test_contents.phpt",
      },
    };
  }

  protected function install(): void {
    parent::install();
    verbose("Creating a phpunit.xml for running the pear tests.\n",
            Options::$verbose);
    $phpunit_xml = <<<XML
<phpunit>
<testsuites>
  <testsuite name="Pear">
    <directory suffix=".phpt">tests</directory>
  </testsuite>
</testsuites>
</phpunit>
XML;
    file_put_contents($this->getTestPath()."/phpunit.xml", $phpunit_xml);
  }

  protected function isInstalled(): bool {
    $extra_files = Set {
      $this->getTestPath()."/phpunit.xml",
      $this->getInstallRoot()."/Console",
      $this->getInstallRoot()."/XML",
      $this->getInstallRoot()."/Structures",
      $this->getInstallRoot()."/Archive",
    };

    if (file_exists($this->getInstallRoot())) {
      // Make sure all the pull requests that have been added along the way
      // are there; otherwise we need a redownload.
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

class Phpbb3 extends Framework {
  public function __construct(string $name) {
    parent::__construct($name, true, TestFindModes::TOKEN);
  }

  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/phpbb3",
      "git_path" => "https://github.com/phpbb/phpbb.git",
      "git_commit" => "b474917ba3fbb26e50a7145fa904efec949f20ce",
      "test_path" => __DIR__."/frameworks/phpbb3",
      "env_vars" => Map {'PHP_BINARY' => get_runtime_build(false, true)},
      // This may work if we increase the timeout. Blacklist for now
      "blacklist" => Set {
        __DIR__."/frameworks/phpbb3/tests/lint_test.php",
      },
    };
  }
}

class PhpMyAdmin extends Framework {
  public function __construct(string $name) {
    parent::__construct($name, true, TestFindModes::TOKEN);
  }

  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/phpmyadmin",
      "git_path" => "https://github.com/phpmyadmin/phpmyadmin.git",
      "git_commit" => "0ce072a3530694bcb0e2e0e3d91874bee6d9a6c4",
      "test_path" => __DIR__."/frameworks/phpmyadmin",
      "config_file" => __DIR__.
                       "/frameworks/phpmyadmin/phpunit.xml.nocoverage",
    };
  }
}

class PHPUnit extends Framework {
  public function __construct(string $name) {
    parent::__construct($name);
  }

  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/phpunit",
      "git_path" => "https://github.com/sebastianbergmann/phpunit.git",
      "git_commit" => "236f65cc97d6beaa8fcb8a27b19bd278f3912677",
      "test_path" => __DIR__."/frameworks/phpunit",
      "test_command" => get_runtime_build()." ".__DIR__.
                        "/frameworks/phpunit/phpunit.php",
      "env_vars" => Map {'PHP_BINARY' => get_runtime_build(false, true)},
      "blacklist" => Set {
        __DIR__."/frameworks/phpunit/Tests/Util/ConfigurationTest.php",
      }
    };
  }
}

class SilverStripe extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      'install_root' => __DIR__.'/frameworks/silverstripe',
      'git_path' => 'https://github.com/silverstripe/'
        . 'silverstripe-installer.git',
      'git_commit' => 'c7fe54d08200e09094c7a6087572cc9d116c5774',
      'test_path' => __DIR__.'/frameworks/silverstripe',
    };
  }

  protected function install(): void {
    parent::install();
    verbose("Installing dependencies.\n", Options::$verbose);

    $dependencies_install_cmd = get_runtime_build()." ".__DIR__.
      "/composer.phar require silverstripe/sqlite3 dev-master";
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

class Slim extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      'install_root' => __DIR__.'/frameworks/Slim',
      // Using a branch from https://github.com/codeguy/Slim to access an
      // upstream hash_hmac fix
      'git_path' => 'https://github.com/elgenie/Slim.git',
      'git_commit' => '1beca31c1f0b0a7bb7747d9367fb07c07e190a8d',
      'test_path' => __DIR__.'/frameworks/Slim',
      'test_command' => get_runtime_build()
        .' -vServer.IniFile='.__DIR__.'/php_notice.ini'
        .' '.__DIR__.'/vendor/bin/phpunit',
    };
  }
}

class Symfony extends Framework {
  public function __construct(string $name) {
    parent::__construct($name, true, TestFindModes::TOKEN);
  }

  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/symfony",
      "git_path" => "https://github.com/JoelMarcey/symfony.git",
      "git_commit" => "4016b4ed7f732211f6093d59913ff84e3cb1d729",
      "test_path" => __DIR__."/frameworks/symfony",
      "env_vars" => Map {'PHP_BINARY' => get_runtime_build(false, true)},
      "blacklist" => Set {
        __DIR__."/frameworks/symfony/src/Symfony/Component/Console".
        "/Tests/Helper/DialogHelperTest.php",
        __DIR__."/frameworks/symfony/src/Symfony/Component/Process".
        "/Tests/SigchildDisabledProcessTest.php",
        __DIR__."/frameworks/symfony/src/Symfony/Component/Process".
        "/Tests/SigchildEnabledProcessTest.php",
        __DIR__."/frameworks/symfony/src/Symfony/Component/Process".
        "/Tests/SimpleProcessTest.php",
      },
    };
  }
}

class Twig extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/Twig",
      "git_path" => "https://github.com/fabpot/Twig.git",
      "git_commit" => "2d012c4a4ae41cdf1682a10b3d567becc38a2d39",
      "test_path" => __DIR__."/frameworks/Twig",
    };
  }
}

class Yii extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/yii",
      "git_path" => "https://github.com/yiisoft/yii.git",
      "git_commit" => "3deee8ee7d67122c21b5938109b37ba570caa761",
      "test_path" => __DIR__."/frameworks/yii/tests",
      "env_vars" => Map {'PHP_BINARY' => get_runtime_build(false, true)},
      "clownylist" => Set {
        // Needs a local memcache server
        __DIR__."/frameworks/yii/tests/framework/caching/CMemCacheTest.php",
      },
    };
  }

  public function clean(): void {
    parent::clean();
    $files = glob($this->getInstallRoot().
                  "/tests/assets/*/CAssetManagerTest.php");
    foreach ($files as $file) {
      verbose("Removing $file\n", Options::$verbose);
      unlink($file);
    }
  }

  protected function install(): void {
    parent::install();
    verbose("Creating a new phpunit.xml for running the yii tests.\n",
            Options::$verbose);
    $phpunit_xml = <<<XML
<phpunit bootstrap="bootstrap.php"
    colors="false"
    convertErrorsToExceptions="true"
    convertNoticesToExceptions="true"
    convertWarningsToExceptions="true"
    stopOnFailure="false">
<testsuites>
  <testsuite name="yii">
    <directory suffix="Test.php">./</directory>
  </testsuite>
</testsuites>
</phpunit>
XML;
    file_put_contents($this->getTestPath()."/phpunit.xml.dist", $phpunit_xml);
    unlink($this->getTestPath()."/phpunit.xml");
  }

  protected function isInstalled(): bool {
    $extra_files = Set {
      $this->getTestPath()."/phpunit.xml.dist",
    };

    if (file_exists($this->getInstallRoot())) {
      // Make sure all the pull requests that have been added along the way
      // are there; otherwise we need a redownload.
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

class Zf2 extends Framework {
  public function __construct(string $name) { parent::__construct($name); }
  protected function getInfo(): Map {
    return Map {
      "install_root" => __DIR__."/frameworks/zf2",
      "git_path" => "https://github.com/JoelMarcey/zf2.git",
      "git_commit" => "b03b54c40fba11eb236e4ab710a1c55973633588",
      "test_path" => __DIR__."/frameworks/zf2/tests",
      "blacklist" => Set {
        __DIR__."/frameworks/zf2/tests/ZendTest/Code/Generator".
        "/ParameterGeneratorTest.php",
        __DIR__."/frameworks/zf2/tests/ZendTest/Code/Generator".
        "/PropertyGeneratorTest.php",
        __DIR__."/frameworks/zf2/tests/ZendTest/Code/Generator".
        "/ValueGeneratorTest.php",
      }
    };
  }
}

class Runner {
  public Framework $framework;
  // Name could be the name of the single test file for a given framework,
  // or the actual framework name if we are running in serial, for example
  public string $name;

  private string $test_information = "";
  private string $error_information = "";
  private string $fatal_information = "";
  private string $diff_information = "";
  private string $stat_information = "";

  private array $pipes = null;
  private resource $process = null;
  private string $actual_test_command = "";

  public function __construct(Framework $f, string $p = "") {
    $this->framework = $f;
    if ($p === "") {
      $this->name = $this->framework->getName();
    } else {
      $this->name = $p;
    }
  }

  public function run(): int {
    chdir($this->framework->getTestPath());
    $ret_val = 0;
    $line = "";
    $post_test = false;
    $pretest_data = true;
    if ($this->initialize()) {
      while (!(feof($this->pipes[1]))) {
        $line = $this->getLine();
        if ($line === null) {
          break;
        }
        if ($this->isBlankLine($line)) {
          continue;
        }
        // e.g. PHPUnit 3.7.28 by Sebastian Bergmann.
        // Even if there are three lines of prologue, this will keep
        // continuing before we call analyzeTest
        if ($this->isPrologue($line)) {
          $pretest_data = false;
          continue;
        }
        // e.g. HipHop Warning: HipHop currently does not support circular
        // reference collection
        // e.g. No headers testing
        // e.g. Please install runkit and enable runkit.internal_override!
        if ($pretest_data) {
          if ($this->checkForWarnings($line)) {
            $this->error_information .= "PRETEST WARNING FOR ".
                                        $this->name.PHP_EOL.$line.PHP_EOL;
            $this->error_information .= $this->getTestRunStr($this->name,
                                                             "RUN TEST FILE: ").
                                        PHP_EOL;
          }
          continue;
        }
        if ($this->isStop($line)) {
          // If we have finished the tests, then we are just printing any error
          // info and getting the final stats
          $this->printPostTestInfo();
          break;
        }
        if (!$pretest_data) {
          // We have gotten through the prologue and any blank lines
          // and we should be at tests now.
          $tn_matches = array();
          if (preg_match($this->framework->getTestNamePattern(), $line,
                         $tn_matches) === 1) {
            // If analyzeTest returns false, then we have most likely
            // hit a fatal. So we bail the run.
            if(!$this->analyzeTest($tn_matches[0])) {
              break;
            }
          } else if ($this->checkForWarnings($line)) {
            // We have a warning after the tests have supposedly started
            // but we really don't have a test to examine.
            // e.g.
            // PHPUnit 3.7.28 by Sebastian Bergmann.
            // The Xdebug extension is not loaded. No code coverage will be gen
            // HipHop Notice: Use of undefined constant DRIZZLE_CON_NONE
            $line = remove_string_from_text($line, __DIR__, "");
            $this->error_information .= PHP_EOL.$line.PHP_EOL;
            $this->error_information .= $this->getTestRunStr($this->name,
                                                             "RUN TEST FILE: ").
                                        PHP_EOL.PHP_EOL;
            continue;
          } else if ($this->checkForFatals($line)) {
            // We have a fatal after the tests have supposedly started
            // but we really don't have a test to examine.
            // e.g.
            // PHPUnit 3.7.28 by Sebastian Bergmann.
            // The Xdebug extension is not loaded. No code coverage will be gen
            // HipHop Fatal error: Undefined function: mysqli_report
            $line = remove_string_from_text($line, __DIR__, "");
            $this->fatal_information .= PHP_EOL.$this->name.
              PHP_EOL.$line.PHP_EOL.PHP_EOL;
            $this->fatal_information .= $this->getTestRunStr($this->name,
                                                             "RUN TEST FILE: ").
                                        PHP_EOL.PHP_EOL;
            break;
          }
        }
      }
      $ret_val = $this->finalize();
      $this->outputData();
    } else {
      error_and_exit("Could not open process to run test ".$this->name.
                     " for framework ".$this->framework->getName(),
                     Options::$csv_only);
    }
    chdir(__DIR__);
    return $ret_val;
  }

  private function analyzeTest(string $test): bool {
    verbose("Analyzing test: ".$test.PHP_EOL, Options::$verbose);
    // If we hit a fatal or something, we will stop the overall test running
    // for this particular test sequence
    $continue_testing = true;
    // We have the test. Now just get the incoming data unitl we find some
    // sort of status data
    do {
      $status = $this->getLine();
      if ($status !== null) {
        // No user specific information in status. Replace with empty string
        $status = remove_string_from_text($status, __DIR__, "");
      }
      if ($status === null) {
        $status = Statuses::UNKNOWN;
        $this->fatal_information .= $test.PHP_EOL.$status.PHP_EOL;
        $this->fatal_information .= $this->getTestRunStr($test,
                                                         "RUN TEST FILE: ").
                                    PHP_EOL.PHP_EOL;
        $this->stat_information = $this->name.PHP_EOL.$status.PHP_EOL;
        $continue_testing = false;
        break;
      } else if ($status === Statuses::TIMEOUT) {
        $this->fatal_information .= $test.PHP_EOL.$status.PHP_EOL;
        $this->fatal_information .= $this->getTestRunStr($test,
                                                         "RUN TEST FILE: ").
                                    PHP_EOL.PHP_EOL;
        $this->stat_information = $this->name.PHP_EOL.$status.PHP_EOL;
        $continue_testing = false;
        break;
      } else if ($this->checkForFatals($status)) {
        $this->fatal_information .= $test.PHP_EOL.$status.PHP_EOL.PHP_EOL;
        $this->fatal_information .= $this->getTestRunStr($test,
                                                         "RUN TEST FILE: ").
                                    PHP_EOL.PHP_EOL;
        $status = Statuses::FATAL;
        $this->stat_information = $this->name.PHP_EOL.$status.PHP_EOL;
        $continue_testing = false;
        break;
      } else if ($this->checkForWarnings($status)) {
        // Warnings are special. We may get one or more warnings, but then
        // a real test status will come afterwards.
        $this->error_information .= $test.PHP_EOL.$status.PHP_EOL.PHP_EOL;
        $this->error_information .= $this->getTestRunStr($test,
                                                         "RUN TEST FILE: ").
                                    PHP_EOL.PHP_EOL;
        continue;
      }
    } while (!feof($this->pipes[1]) &&
           preg_match(PHPUnitPatterns::$status_code_pattern,
                      $status) === 0);
    // Test names should have all characters before and including __DIR__
    // removed, so that specific user info is not added
    $test = rtrim($test, PHP_EOL);
    $test = remove_string_from_text($test, __DIR__, null);
    $this->test_information .= $test.PHP_EOL;
    $this->processStatus($status, $test);

    return $continue_testing;
  }

  private function processStatus(string $status, string $test): void {
    // May have this if we reached the end of the file or if something
    // wasn't printed out in optimized mode that may have been printed
    // out in debug mode
    if ($status === "" || $status === null) {
      $status = Statuses::UNKNOWN;
    } else if ($status !== Statuses::UNKNOWN && $status !== Statuses::TIMEOUT &&
               $status !== Statuses::FATAL) {
      // Otherwise we have, Fail, Error, Incomplete, Skip, Pass (.)
      // First Char In case we had "F 252 / 364 (69 %)"
      $status = $status[0];
    }

    $this->test_information .= $status.PHP_EOL;

    if ($this->framework->getCurrentTestStatuses() !== null &&
        $this->framework->getCurrentTestStatuses()->containsKey($test)) {
      if ($status === $this->framework->getCurrentTestStatuses()[$test]) {
        // FIX: posix_isatty(STDOUT) was always returning false, even
        // though can print in color. Check this out later.
        verbose(Colors::GREEN.Statuses::PASS.Colors::NONE, !Options::$csv_only);
      } else {
        // Red if we go from pass to something else
        if ($this->framework->getCurrentTestStatuses()[$test] === '.') {
          verbose(Colors::RED.Statuses::FAIL.Colors::NONE, !Options::$csv_only);
        // Green if we go from something else to pass
        } else if ($status === '.') {
          verbose(Colors::GREEN.Statuses::FAIL.Colors::NONE,
                  !Options::$csv_only);
        // Blue if we go from something "faily" to something "faily"
        // e.g., E to I or F
        } else {
          verbose(Colors::BLUE.Statuses::FAIL.Colors::NONE,
                  !Options::$csv_only);
        }
        verbose(PHP_EOL."Different status in ".$this->framework->getName().
                " for test ".$test." was ".
                $this->framework->getCurrentTestStatuses()[$test].
                " and now is ".$status.PHP_EOL, !Options::$csv_only);
        $this->diff_information .= "----------------------".PHP_EOL;
        $this->diff_information .= $test.PHP_EOL.PHP_EOL;
        $this->diff_information .= $this->getTestRunStr($test,
                                                        "RUN TEST FILE: ").
                                   PHP_EOL.PHP_EOL;
        $this->diff_information .= "EXPECTED: ".$this->framework->
                                   getCurrentTestStatuses()[$test].PHP_EOL;
        $this->diff_information .= ">>>>>>>".PHP_EOL;
        $this->diff_information .= "ACTUAL: ".$status.PHP_EOL.PHP_EOL;
      }
    } else {
      // This is either the first time we run the unit tests, and all pass
      // because we are establishing a baseline. OR we have run the tests
      // before, but we are having an issue getting to the actual tests
      // (e.g., yii is one test suite that has behaved this way).
      if ($this->framework->getCurrentTestStatuses() !== null) {
        verbose(Colors::LIGHTBLUE.Statuses::FAIL.Colors::NONE,
                !Options::$csv_only);
        verbose(PHP_EOL."Different status in ".$this->framework->getName().
                " for test ".$test.PHP_EOL,!Options::$csv_only);
        $this->diff_information .= "----------------------".PHP_EOL;
        $this->diff_information .= "Maybe haven't see this test before: ".
                                   $test.PHP_EOL.PHP_EOL;
        $this->diff_information .= $this->getTestRunStr($test,
                                                        "RUN TEST FILE: ").
                                   PHP_EOL.PHP_EOL;
      } else {
        verbose(Colors::GRAY.Statuses::PASS.Colors::NONE, !Options::$csv_only);
      }
    }
  }

  private function getLine(): ?string {
    if (feof($this->pipes[1])) {
      return null;
    }
    if (!$this->checkReadStream()) {
      return Statuses::TIMEOUT;
    }
    $line = stream_get_line($this->pipes[1], 4096, PHP_EOL);
    // No more data
    if ($line === false || $line === null || strlen($line) === 4096) {
      return null;
    }
    $line = remove_color_codes($line);
    return $line;
  }

  // Post test information are error/failure information and the final passing
  // stats for the test
  private function printPostTestInfo(): void {
    $prev_line = null;
    $final_stats = null;
    $matches = array();
    $post_stat_fatal = false;

    // Throw out any initial blank lines
    do {
      $line = $this->getLine();
    } while ($line === "" && $line !== null);

    // Now that we have our first non-blank line, print out the test information
    // until we have our final stats
    while ($line !== null) {
      // Don't print out any of the PHPUnit Patterns to the errors file.
      // Just print out pertinent error information.
      //
      // There was 1 failure:  <---- Don't print
      // <blank line>
      // 1) Assetic\Test\Asset\HttpAssetTest::testGetLastModified <---- print
      if (preg_match(PHPUnitPatterns::$tests_ok_skipped_inc_pattern,
                     $line) === 1 ||
          preg_match(PHPUnitPatterns::$num_errors_failures_pattern,
                     $line) === 1 ||
          preg_match(PHPUnitPatterns::$failures_header_pattern,
                     $line) === 1 ||
          preg_match(PHPUnitPatterns::$num_skips_inc_pattern,
                     $line) === 1) {
        do {
          // throw out any blank lines after these pattern
          $line = $this->getLine();
        } while ($line === "" && $line !== null);
        continue;
      }

      // If we hit what we think is the final stats based on the pattern of the
      // line, make sure this is the case. The final stats will generally be
      // the last line before we hit null returned from line retrieval. The
      // only cases where this would not be true is if, for some rare reason,
      // stat information is part of the information provided for a
      // given test error -- or -- we have hit a fatal at the very end of
      // running PHPUnit. For that fatal case, we handle that a bit differently.
      if (preg_match(PHPUnitPatterns::$tests_ok_pattern, $line) === 1 ||
          preg_match(PHPUnitPatterns::$tests_failure_pattern, $line) === 1 ||
          preg_match(PHPUnitPatterns::$no_tests_executed_pattern,
                     $line) === 1) {
        $prev_line = $line;
        $line = $this->getLine();
        if ($line === null) {
          $final_stats = $prev_line;
          break;
        } else if ($line === "") {
          // FIX ME: The above $line === null check is all I should need, but
          // but getLine() is not cooperating. Not sure if getLine() problem or
          // a PHPUnit output thing, but even when I am at the final stat line
          // pattern, sometimes it takes me two getLine() calls to hit
          // $line === null because I first get $line === "".
          // So...save the current position. Read ahead. If null, we are done.
          // Otherwise, print $prev_line, go back to where we were and the
          // current blank line now stored in $line, will be printed down
          // below
          $curPos = ftell($this->pipes[1]);
          if ($this->getLine() === null) {
            $final_stats = $prev_line;
            break;
          } else {
            $this->error_information .= $prev_line.PHP_EOL;
            fseek($this->pipes[1], $curPos);
          }
        } else if ($this->checkForFatals($line) ||
                   $this->checkForWarnings($line)) {
        // Sometimes when PHPUnit is done printing its post test info, hhvm
        // fatals. This is not good, but it currently happens nonetheless. Here
        // is an example:
        //
        // FAILURES!
        // Tests: 3, Assertions: 9, Failures: 2. <--- EXPECTED LAST LINE (STATS)
        // Core dumped: Segmentation fault  <--- But, we can get this and below
        // /home/joelm/bin/hhvm: line 1: 28417 Segmentation fault
          $final_stats = $prev_line;
          $post_stat_fatal = true;
          break;
        } else {
          $this->error_information .= $prev_line.PHP_EOL;
        }
      }

      $this->error_information .= $line.PHP_EOL;
      if (preg_match($this->framework->getTestNamePattern(), $line,
                     $matches) === 1) {
        $print_blanks = true;
        $this->error_information .= PHP_EOL.
                                    $this->getTestRunStr($matches[0],
                                                         "RUN TEST FILE: ").
                                    PHP_EOL.PHP_EOL;
      }
      $line = $this->getLine();
    }

    if ($post_stat_fatal) {
      $this->fatal_information .= "POST-TEST FATAL/WARNING FOR ".
                                  $this->name.PHP_EOL;
      $this->fatal_information .= PHP_EOL.
                                  $this->getTestRunStr($this->name,
                                                       "RUN TEST FILE: ").
                                  PHP_EOL.PHP_EOL;
      while ($line !== null) {
        $this->fatal_information .= $line.PHP_EOL;
        $line = $this->getLine();
      }
      // Add a newline to the fatal file if we had a post-test fatal for better
      // visual
      $this->fatal_information .= PHP_EOL;
    }

    // If we have no final stats, assume some sort of fatal for this test.
    // If we have "No tests executed", assume a skip
    // Otherwise, print the final stats.
    $this->stat_information = $this->name.PHP_EOL;
    if ($final_stats === null) {
      $this->stat_information .= Statuses::FATAL.PHP_EOL;
    } else if (preg_match(PHPUnitPatterns::$no_tests_executed_pattern,
                          $final_stats) === 1) {
      $this->stat_information .= Statuses::SKIP.PHP_EOL;
    } else {
      $this->stat_information .= $final_stats.PHP_EOL;
    }
  }

  private function isStop(string $line) {
    if (preg_match(PHPUnitPatterns::$stop_parsing_pattern, $line) === 1) {
      return true;
    }
    return false;
  }

  private function isPrologue(string $line) {
    if (preg_match(PHPUnitPatterns::$header_pattern, $line) === 1 ||
        preg_match(PHPUnitPatterns::$config_file_pattern, $line) === 1 ||
        preg_match(PHPUnitPatterns::$xdebug_pattern, $line) === 1) {
      return true;
    }
    return false;
  }

  private function isBlankLine(string $line): bool {
    if ($line === "" || $line === PHP_EOL) {
      return true;
    }
    return false;
  }

  private function initialize(): bool {
    $this->actual_test_command = $this->framework->getTestCommand($this->name);
    verbose("Command: ".$this->actual_test_command."\n", Options::$verbose);

    $descriptorspec = array(
      0 => array("pipe", "r"),
      1 => array("pipe", "w"),
      2 => array("pipe", "w"),
    );

    $env = $_ENV;
    // Use the proxies in case the test needs access to the outside world
    $env = array_merge($env, ProxyInformation::$proxies->toArray());
    if ($this->framework->getEnvVars() !== null) {
      $env = array_merge($env, $this->framework->getEnvVars()->toArray());
    }
    $this->process = proc_open($this->actual_test_command, $descriptorspec,
                               $this->pipes, $this->framework->getTestPath(),
                               $env);
    return is_resource($this->process);
  }

  private function finalize(): int {
    fclose($this->pipes[0]);
    fclose($this->pipes[1]);
    fclose($this->pipes[2]);

    if (proc_close($this->process) === -1) {
      return -1;
    }

    return 0;
  }

  private function checkReadStream(): bool {
    $r = array($this->pipes[1]);
    $w = null;
    $e = null;
    $s = stream_select($r, $w, $e, Options::$timeout);
    // If stream_select returns 0, then there is no more data or we have
    // timed out. If it returns false, then something else bad happened.
    if ($s === 0 || $s === false) {
      return false;
    }
    return true;
  }

  private function outputData(): void {
    file_put_contents($this->framework->getOutFile(), $this->test_information,
                      FILE_APPEND | LOCK_EX);
    file_put_contents($this->framework->getErrorsFile(),
                      $this->error_information, FILE_APPEND | LOCK_EX);
    file_put_contents($this->framework->getDiffFile(), $this->diff_information,
                      FILE_APPEND | LOCK_EX);
    file_put_contents($this->framework->getStatsFile(), $this->stat_information,
                      FILE_APPEND | LOCK_EX);
    file_put_contents($this->framework->getFatalsFile(),
                      $this->fatal_information, FILE_APPEND | LOCK_EX);

  }

  private function checkForFatals(string $line): bool {
    if (preg_match(PHPUnitPatterns::$hhvm_fatal_pattern, $line) === 1) {
      return true;
    }
    return false;
  }

  private function checkForWarnings(string $line): bool {
    if (preg_match(PHPUnitPatterns::$hhvm_warning_pattern, $line) === 1) {
      return true;
    }
    if (preg_match(PHPUnitPatterns::$phpunit_exception_with_hhvm_warning,
                   $line) === 1) {
      return true;
    }
    return false;
  }

  private function getTestRunStr(string $test, string $prologue = "",
                                 string $epilogue = ""): string {
    $test_run = $prologue;
    $test_run .= " cd ".$this->framework->getTestPath()." && ";
    // If the test that is coming in to this function is an individual test,
    // as opposed to a file, then we can use the --filter option to make the
    // run string have even more specificity.
    if (preg_match($this->framework->getTestNamePattern(), $test)) {
      // If we are running this framework with individual test mode
      // (e.g., --by-test), then --filter already exists. We also don't want to
      // add --filter to .phpt style tests (e.g. Pear).
      if (strpos($this->actual_test_command, "--filter") === false &&
          strpos($test, ".phpt") === false) {
        // The string after the last space in actual_test_command is
        // the file that is run in phpunit. Remove the file and replace
        // with --filter <individual test>. This will also get rid of any
        // 2>&1 that may exist as well, which we do not want.
        //
        // e.g.,
        // hhvm -v Eval.Jit=true phpunit --debug 'ConverterTest.php'
        // to
        // hhvm -v Eval.Jit=true phpunit --debug 'ConverterTest::MyTest'
        $t = rtrim(str_replace("2>&1", "", $this->actual_test_command));
        $lastspace = strrpos($t, ' ');
        $t = substr($this->actual_test_command, 0, $lastspace);
        // For --filter, the namespaces need to be separated by \\
        $test = str_replace("\\", "\\\\", $test);
        $t .= " --filter '".$test."'";
        $test_run .= $t;
      } else if (!$this->framework->isParallel()) {
      // If a framework is not being run in parallel (e.g., it is being run like
      // normal phpunit for the entire framework), then the actual_test_command
      // would not contain the individual test by default. It is being run like
      // this, for example, from the test root directory:
      //
      // hhvm phpunit
      //
      // Pear is a current example of this behavior.
        $test_run .= rtrim(str_replace("2>&1", "", $this->actual_test_command));
        // Re-add __DIR__ if not there so we have a full test path to run
        if (strpos($test, __DIR__) !== 0) {
          $test_run .= " ".__DIR__."/".$test;
        } else {
          $test_run .= " ".$test;
        }

      } else {
        $test_run .= rtrim(str_replace("2>&1", "", $this->actual_test_command));
      }
    } else {
    // $test is not a XXX::YYY style test, but is instead a file that is already
    // part of the actual_test_comand
      $test_run .= rtrim(str_replace("2>&1", "", $this->actual_test_command));
    }
    $test_run .= $epilogue;
    return $test_run;
  }
}

function prepare(Set $available_frameworks, Vector $passed_frameworks): Vector {
  get_unit_testing_infra_dependencies();

  if (Options::$all) {
    // At this point, $framework_names should be empty if we are in --all mode.
    if (!($passed_frameworks->isEmpty())) {
      error_and_exit("Do not specify both --all and individual frameworks to ".
                     "run at same time.\n");
    }
    // Test all frameworks
    $passed_frameworks = $available_frameworks->toVector();
  } else if (Options::$allexcept) {
    // Run all the frameworks, but the ones we listed.
    $passed_frameworks  = Vector::fromItems(array_diff(
                                              $available_frameworks->toVector(),
                                              $passed_frameworks));
  } else if (count($passed_frameworks) === 0) {
    error_and_exit(usage());
  }

  // So it is easier to keep tabs on our progress when running ps or something.
  // Since I get all the tests of a framework by foreaching over the frameworks
  // vector, and then append those tests to a tests vector and then foreach the
  // test vector to bucketize them, this will allow us to basically run the
  // framework tests alphabetically.
  sort($passed_frameworks);
  $frameworks = Vector {};
  foreach ($passed_frameworks as $name) {
    $name = trim(strtolower($name));
    if ($available_frameworks->contains($name)) {
      $uname = ucfirst($name);
      $framework = new $uname($name);
      $frameworks[] = $framework;
    }
  }

  if (count($frameworks) === 0) {
    error_and_exit(usage());
  }

  return $frameworks;
}

function fork_buckets(Traversable $data, Callable $callback): int {
  $num_threads = min(count($data), num_cpus() + 1);

  // Create some data buckets proportional to the number of threads
  $data_buckets = array();
  $i = 0;
  foreach ($data as $d) {
    $data_buckets[$i][] = $d;
    $i = ($i + 1) % $num_threads;
  }

  $children = Vector {};
  for ($i = 0; $i < $num_threads; $i++) {
    $pid = pcntl_fork();
    if ($pid === -1) {
      error_and_exit('Issues creating threads for data');
    } else if ($pid) {
      $children[] = $pid;
    } else {
      exit($callback($data_buckets[$i]));
    }
  }

  $thread_ret_val = 0;
  $status = -1;
  foreach($children as $child) {
    pcntl_waitpid($child, $status);
    $thread_ret_val |= pcntl_wexitstatus($status);
  }

  return $thread_ret_val;
}

function run_tests(Vector $frameworks): void {
  if (count($frameworks) === 0) {
    error_and_exit("No frameworks available on which to run tests");
  }

  /***********************************
   *  Initial preparation
   **********************************/
  $summary_file = tempnam("/tmp", "oss-fw-test-summary");
  $all_tests = Vector {};

  foreach($frameworks as $framework) {
    $framework->clean();
    if (file_exists($framework->getExpectFile())) {
      $framework->prepareCurrentTestStatuses(
                                        PHPUnitPatterns::$status_code_pattern,
                                        PHPUnitPatterns::$stop_parsing_pattern);
      verbose(Colors::YELLOW.$framework->getName().Colors::NONE.": running. ".
              "Comparing against ".count($framework->getCurrentTestStatuses()).
              " tests\n", !Options::$csv_only);
      $run_msg = "Comparing test suite with previous run. ";
      $run_msg .= Colors::GREEN."Green . (dot) ".Colors::NONE;
      $run_msg .= "means test result same as previous. ";
      $run_msg .= "A ".Colors::GREEN." green F ".Colors::NONE;
      $run_msg .= "means we have gone from fail to pass. ";
      $run_msg .= "A ".Colors::RED." red F ".Colors::NONE;
      $run_msg .= "means we have gone from pass to fail. ";
      $run_msg .= "A ".Colors::BLUE." blue F ".Colors::NONE;
      $run_msg .= "means we have gone from one type of fail to another type ";
      $run_msg .= "of fail (e.g., F to E, or I to F). ";
      $run_msg .= "A ".Colors::LIGHTBLUE." light blue F ".Colors::NONE;
      $run_msg .= "means we are having trouble accessing the tests from the ";
      $run_msg .= "expected run and can't get a proper status".PHP_EOL;
      verbose($run_msg, Options::$verbose);
    } else {
      verbose("Establishing baseline statuses for ".$framework->getName().
              " with gray dots...\n", !Options::$csv_only);
    }

    // If we are running the tests for the framework in parallel, then let's
    // get all the test for that framework and add each to our tests
    // vector; otherwise, we are just going to add the framework to run
    // serially and use its global phpunit test run command to run the entire
    // suite.
    if ($framework->isParallel()) {
      foreach($framework->getTests() as $test) {
        $st = new Runner($framework, $test);
        $all_tests->add($st);
      }
    } else {
      $st = new Runner($framework);
      $all_tests->add($st);
    }
  }

  /*************************************
   * Run the test suite
   ************************************/
  verbose("Beginning the unit tests.....\n", !Options::$csv_only);
  if (count($all_tests) === 0) {
    error_and_exit("No tests found to run");
  }

  fork_buckets(
    $all_tests,
    function($bucket) {
      return run_test_bucket($bucket);
    }
  );

  /****************************************
  * All tests complete. Create results for
  * the framework that just ran.
  ****************************************/
  $all_tests_success = true;
  $diff_frameworks = Vector {};
  foreach ($frameworks as $framework) {
    $pct = $framework->getPassPercentage();
    $encoded_result = json_encode(array($framework->getName() => $pct));
    if (!(file_exists($summary_file))) {
      file_put_contents($summary_file, $encoded_result);
    } else {
      $file_data = file_get_contents($summary_file);
      $decoded_results = json_decode($file_data, true);
      $decoded_results[$framework->getName()] = $pct;
      file_put_contents($summary_file, json_encode($decoded_results));
    }

    // If the first baseline run, make both the same. Otherwise, see if we have
    // a diff file. If not, then all is good. If not, thumbs down because there
    // was a difference between what we ran and what we expected.
    Framework::sortFile($framework->getOutFile());
    if (!file_exists($framework->getExpectFile())) {
      copy($framework->getOutFile(), $framework->getExpectFile());
    } else if (file_get_contents($framework->getExpectFile()) !==
               file_get_contents($framework->getOutFile())) {
      $all_tests_success = false;
    }

    if (filesize($framework->getDiffFile()) > 0) {
      $diff_frameworks[] = $framework;
    }
  }

  if ($all_tests_success) {
    $msg = "\nAll tests ran as expected.\n\n".<<<THUMBSUP
          _
         /(|
        (  :
       __\  \  _____
     (____)  -|
    (____)|   |
     (____).__|
      (___)__.|_____
THUMBSUP
."\n";
   verbose($msg, !Options::$csv_only);
  } else {
    $msg = "\nAll tests did not run as expected. Either some statuses were ".
           "different or the number of tests run didn't match the number of ".
           "tests expected to run\n\n".<<<THUMBSDOWN
      ______
     (( ____ \-
     (( _____
     ((_____
     ((____   ----
          /  /
         (_((
THUMBSDOWN
."\n";
    verbose($msg, !Options::$csv_only);
  }

  // Print the diffs
  if (!Options::$csv_only) {
    foreach($diff_frameworks as $framework) {
      print_diffs($framework);
    }
  }

  // Print out summary information
  print_summary_information($summary_file);
}

function run_test_bucket(array $test_bucket): int {
  $result = 0;
  foreach($test_bucket as $test) {
    $result = $test->run();
  }
  return $result;
}

function get_unit_testing_infra_dependencies(): void {
  // Install composer.phar. If it exists, but it is nearing that
  // 30 day old mark, resintall it anyway.
  if (!(file_exists(__DIR__."/composer.phar")) ||
      (time() - filectime(__DIR__."/composer.phar")) >= 29*24*60*60) {
    verbose("Getting composer.phar....\n", !Options::$csv_only);
    unlink(__DIR__."/composer.phar");
    $comp_url = "http://getcomposer.org/composer.phar";
    $get_composer_command = "wget ".$comp_url." -P ".__DIR__." 2>&1";
    $ret = run_install($get_composer_command, __DIR__,
                       ProxyInformation::$proxies);
    if ($ret !== 0) {
      error_and_exit("Could not download composer. Script stopping\n");
    }
  }

  // Quick hack to make sure we get the latest phpunit binary from composer
  $md5_file = __DIR__."/composer.json.md5";
  $json_file = __DIR__."/composer.json";
  $vendor_dir = __DIR__."/vendor";
  $lock_file = __DIR__."/composer.lock";
  if (!file_exists($md5_file) ||
      file_get_contents($md5_file) !== md5($json_file)) {
    verbose("\nUpdated composer.json found. Updating phpunit binary.\n",
            !Options::$csv_only);
    if (file_exists($vendor_dir)) {
      remove_dir_recursive($vendor_dir);
    }
    unlink($lock_file);
    file_put_contents($md5_file, md5($json_file));
  }

  // Install phpunit from composer.json located in __DIR__
  $phpunit_binary = __DIR__."/vendor/bin/phpunit";
  if (!(file_exists($phpunit_binary))) {
    verbose("\nDownloading PHPUnit in order to run tests. There may be an ".
            "output delay while the download begins.\n", !Options::$csv_only);
    // Use the timeout to avoid curl SlowTimer timeouts and problems
    $phpunit_install_command = get_runtime_build()." ".
                               "-v ResourceLimit.SocketDefaultTimeout=30 ".
                               __DIR__.
                               "/composer.phar install --dev --verbose 2>&1";
    $ret = run_install($phpunit_install_command, __DIR__,
                       ProxyInformation::$proxies);
    if ($ret !== 0) {
      error_and_exit("Could not install PHPUnit. Script stopping\n");
    }
  }

}

// This will run processes that will get the test infra dependencies
// (e.g. PHPUnit), frameworks and framework dependencies.
function run_install(string $proc, string $path, ?Map $env): ?int
{
  verbose("Running: $proc\n", Options::$verbose);
  $descriptorspec = array(
    0 => array("pipe", "r"),
    1 => array("pipe", "w"),
    2 => array("pipe", "w"),
  );

  $env_arr = null; // $_ENV will passed in by default if this is null
  if ($env !== null) {
    $env_arr = array_merge($_ENV, $env->toArray());
  }
  $pipes = null;
  $process = proc_open($proc, $descriptorspec, $pipes, $path, $env_arr);
  if (is_resource($process)) {
    fclose($pipes[0]);
    $start_time = microtime(true);
    while ($line = fgets($pipes[1])) {
      verbose("$line", Options::$verbose);
      if ((microtime(true) - $start_time) > 1) {
        verbose(".", !Options::$verbose && !Options::$csv_only);
        $start_time = microtime(true);
      }
    }
    verbose(stream_get_contents($pipes[2]), Options::$verbose);
    fclose($pipes[1]);
    $ret = proc_close($process);
    verbose("Returned status $ret\n", Options::$verbose);
    return $ret;
  }
  verbose("Couldn't proc_open: $proc\n", Options::$verbose);
  return null;
}

function print_diffs(Framework $framework): void {
  $diff = $framework->getDiffFile();
  // The file may not exist or the file may not have anything in it
  // since there is no diff (i.e., all tests for that particular
  // framework ran as expected). Either way, don't print anything
  // out for those cases.
  if (file_exists($diff) &&
     ($contents = file_get_contents($diff)) !== "") {
    print PHP_EOL."********* ".strtoupper($framework->getName()).
          " **********".PHP_EOL;
    print $contents;
  }
}

function print_summary_information(string $summary_file): void {
  if (file_exists($summary_file)
      && ($contents = file_get_contents($summary_file)) !== "") {
    $file_data = file_get_contents($summary_file);
    $decoded_results = json_decode($file_data, true);
    ksort($decoded_results);

    if (Options::$csv_only) {
      if (Options::$csv_header) {
        $print_str = str_pad("date,", 20);
        foreach ($decoded_results as $key => $value) {
          $print_str .= str_pad($key.",", 20);
        }
        // Get rid of the the last spaces and comma
        $print_str = rtrim($print_str);
        $print_str = rtrim($print_str, ",");
        $print_str .= PHP_EOL;
        print $print_str;
      }
      $print_str = str_pad(date("Y/m/d-G:i:s").",", 20);
      foreach ($decoded_results as $key => $value) {
        $print_str .= str_pad($value.",", 20);
      }
      // Get rid of the the last spaces and comma
      $print_str = rtrim($print_str);
      $print_str = rtrim($print_str, ",");
      $print_str .= PHP_EOL;
      print $print_str;
    } else {
      print PHP_EOL."ALL TESTS COMPLETE!".PHP_EOL;
      print "SUMMARY:".PHP_EOL;
      foreach ($decoded_results as $key => $value) {
        print $key."=".$value.PHP_EOL;
      }
      print PHP_EOL;
      print "To run differing tests (if they exist), see above for the".PHP_EOL;
      print "commands or the results/.diff file. To run erroring or".PHP_EOL;
      print "fataling tests see results/.errors and results/.fatals".PHP_EOL;
      print "files, respectively".PHP_EOL;
    }
  } else {
      verbose("\nNO SUMMARY INFO AVAILABLE!\n", !Options::$csv_only);
  }
}

function help(): void {
  $intro = <<<INTRO
  oss_framework_test_script

    This script runs one or more open source (oss) framework unit tests
    via PHPUnit. Run one or multiple tests by explicitly naming them at the
    command line, or specify --all to run all available tests.

    You will see various forms of output. The first time running a test suite
    for a framework, gray dots will appear to set the baseline for future runs.
    On subsequent runs, you will see green dots for tests that have the same
    status as the previous run. If something changes, you will see a red F if
    a test went from pass to something else. You will see a green F if a test
    went from something else to pass. You will see a blue F if a test stayed
    in the failing range, but went from something like E to I or F to S.

    The summary for a test show the overall pass percentage of the unit
    test suite, irrespective of previous runs. The output and diff files for
    a test suite will show what tests pass or fail, and have why they failed.

INTRO;

  $examples = <<<EXAMPLES
  Usage:

    # Run all framework tests.
    % hhvm run.php --all

    # Run all framework tests using zend for alternative options for downloads.
    % hhvm run.php --all --zend ~/zend55/bin/php

    # Run all framework tests forcing the download of all the frameworks and
    # creating new expected output files for all of the frameworks
    % hhvm run.php --all --redownload --record

    # Run all framework tests with a timeout per individual test (in secs).
    % hhvm run.php --all --timeout 30

    # Run one test.
    % hhvm run.php composer

    # Run multiple tests.
    % hhvm run.php composer assetic paris

    # Run all tests except a few.
    % hhvm run.php --allexcept pear symfony

    # Run multiple tests with timeout for each individual test (in seconds).
    # Tests must come after the -- options
    % hhvm run.php --timeout 30 composer assetic

    # Run multiple tests with timeout for each individual test (in seconds) and
    # with verbose messages. Tests must come after the -- options.
    % hhvm run.php --timeout 30 --verbose composer assetic

    # Run all the tests, but only produce a machine readable csv
    # for data extraction into entities like charts.
    % hhvm run.php --all --csv

    # Run all the tests, but only produce a machine readable csv
    # for data extraction into entities like charts, including a header row.
    % hhvm run.php --all --csv --csvheader

    # Display help.
    % hhvm run.php --help


EXAMPLES;

  $run_msg = "When comparing test suites with previous runs: ".PHP_EOL;
  $run_msg .= Colors::GREEN."Green . (dot) ".Colors::NONE;
  $run_msg .= "means test result same as previous. ".PHP_EOL;
  $run_msg .= "A ".Colors::GREEN." green F ".Colors::NONE;
  $run_msg .= "means we have gone from fail to pass. ".PHP_EOL;
  $run_msg .= "A ".Colors::RED." red F ".Colors::NONE;
  $run_msg .= "means we have gone from pass to fail. ".PHP_EOL;
  $run_msg .= "A ".Colors::BLUE." blue F ".Colors::NONE;
  $run_msg .= "means we have gone from one type of fail to another type ";
  $run_msg .= "of fail (e.g., F to E, or I to F). ".PHP_EOL;
  $run_msg .= "A ".Colors::LIGHTBLUE." light blue F ".Colors::NONE;
  $run_msg .= "means we are having trouble accessing the tests from the ";
  $run_msg .= "expected run and can't get a proper status.".PHP_EOL;

  display_help($intro, oss_test_option_map());
  print $examples;
  print $run_msg;

}

function usage(): string {
  $msg = "Specify frameworks to run, use --all or use --allexcept. ";
  $msg .= "Available frameworks are: ".PHP_EOL;
  $msg .= implode(PHP_EOL, get_subclasses_of("Framework")->toArray());
  return $msg;
}

function oss_test_option_map(): OptionInfoMap {
  return Map {
    'help'                => Pair {'h', "Print help message"},
    'all'                 => Pair {'a', "Run tests of all frameworks. The ".
                                        "frameworks to be run are hardcoded ".
                                        "in a Map in this code."},
    'allexcept'           => Pair {'e', "Run all tests of all frameworks ".
                                        "except for the ones listed. The ".
                                        "tests must be at the end of the ".
                                        "command argument list."},
    'timeout:'            => Pair {'',  "The maximum amount of time, in secs, ".
                                        "to allow a individual test to run.".
                                        "Default is 60 seconds."},
    'verbose'             => Pair {'v', "For a lot of messages about what is ".
                                        "going on."},
    'zend:'               => Pair {'',  "Use zend to run the tests ".
                                        "Currently, zend must be installed ".
                                        "and the path to the zend binary".
                                        "specified."},
    'redownload'          => Pair {'',  "Forces a redownload of the framework ".
                                        "code and dependencies."},
    'record'              => Pair {'',  "Forces a new expect file for the ".
                                        "framework test suite"},
    'csv'                 => Pair {'',  "Just create the machine readable ".
                                        "summary CSV for parsing and chart ".
                                        "display."},
    'csvheader'           => Pair {'',  "Add a header line for the summary ".
                                        "CSV which includes the framework ".
                                        "names."},
    'by-file'             => Pair {'f',  "DEFAULT: Run tests for a framework ".
                                         "on a per test file basis, as ".
                                         "opposed to a an individual test ".
                                         "basis."},
    'by-single-test'      => Pair {'s',  "Run tests for a framework on a ".
                                         "individual test file basis, as ".
                                         "opposed to a an individual test ".
                                         "basis."},
  };
}

function main(array $argv): void {
  $options = parse_options(oss_test_option_map());
  if ($options->containsKey('help')) {
    return help();
  }
  $available_frameworks = get_subclasses_of("Framework")->toSet();
  // Parse other possible options out in run()
  $passed_frameworks = Options::parse($options, $argv);
  $frameworks = prepare($available_frameworks, $passed_frameworks);
  run_tests($frameworks);
}

main($argv);

function get_runtime_build(bool $with_jit = true,
                           bool $use_php = false): string {
  $build = "";

  // FIX: Should we try to install a vanilla zend binary here instead of
  // relying on user to specify a path? Should we try to determine if zend
  // is already installed via a $PATH variable?
  if (Options::$zend_path !== null) {
    if (!file_exists(Options::$zend_path)) {
      error_and_exit("Zend build does not exists. Are you sure your path is ".
                     "right?");
    }
    $build = Options::$zend_path;
  } else {
    $fbcode_root_dir = __DIR__.'/../../..';
    $oss_root_dir = __DIR__.'/../..';
    // See if we are using an internal development build
    if ((file_exists($fbcode_root_dir."/_bin"))) {
      $build .= $fbcode_root_dir;
      if (!$use_php) {
        $build .= "/_bin/hphp/hhvm/hhvm";
      } else {
        $build .= "/_bin/hphp/hhvm/php";
      }
    // Maybe we are in OSS land trying this script
    } else if (file_exists($oss_root_dir."/hhvm")) {
      // Pear won't run correctly unless a 'php' executable exists.
      // This may be a Pear thing, a PHPUnit running phpt thing, or
      // or something else. Until we know for sure, let's just create
      // a php symlink to hhvm
      symlink($oss_root_dir."/hhvm/hhvm", $oss_root_dir."/hhvm/php");

      $build .= $oss_root_dir."/hhvm";
      if (!$use_php) {
        $build .= "/hhvm";
      } else {
        $build .= "/php";
      }
    } else {
      error_and_exit("HHVM build doesn't exist. Did you build yet?");
    }
    if (!$use_php) {
      $repo_loc = tempnam('/tmp', 'framework-test');
      $repo_args = " -v Repo.Local.Mode=-- -v Repo.Central.Path=".$repo_loc;
      $build .= $repo_args;
      $build .= " --config ".__DIR__."/config.hdf";
      $build .= " -v Server.IniFile=".__DIR__."/php.ini";
    }
    if ($with_jit) {
      $build .= " -v Eval.Jit=true";
    }
  }
  return $build;
}

function error_and_exit(string $message, bool $to_file = false): void {
  if ($to_file) {
    file_put_contents(Options::$script_errors_file, basename(__FILE__).": ".
                      $message.PHP_EOL, FILE_APPEND);
  } else {
    echo basename(__FILE__).": ".$message.PHP_EOL;
  }
  exit(1);
}
