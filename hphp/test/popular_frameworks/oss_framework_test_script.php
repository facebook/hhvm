#!/usr/local/bin/php
<?hh
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
 *   - Run a single test, all of the tests or a custom set of multiple tests.
 *     Currently, all of the available tests live in the code itself. See the
 *     help for the syntax on how to run the tests in each available mode.
 *
 *   - Multiple tests are run in separate processes, making the entire testing
 *     process a bit faster. This also helps us prepare for incorporating this
 *     with our official test suite.
 *
 *   - The creation (and appending to) a summary file that lists all tests run
 *     and the pass percentage (or fatal) of each test. The impetus for this
 *     file is the "OSS Parity" snapshot on our team TV screen.
 *
 *   - Raw statistics for each test run are put in a temporary file for
 *     examination.
 *
 *   - Timeout option for running tests. Thre is a default of 30 minutes to
 *     run each test, but this can be shortened or lengthened as desired.
 *
 * Future enhancements:
 *
 *   - Integreate the testing with our current "test/run" style infrastructure
 *     for official pass/fail statistics when we have diffs.
 *
 *   - Enhance the data output by the script to include "diff" type information
 *     about why a passing percentage is different from a previous run,
 *     particularly from a regression perspective. For example, what tests
 *     caused the regression.
 *
 *   - Special case frameworks that don't use PHPUnit (e.g. Thinkup).
 *
 *   - Decide whether proxy-based code needs to be solidified better or whether
 *     we should just add the frameworks to our repo officially.
 *
 * Version: 1.0
 *
 *
*/
$FBCODE_ROOT = __DIR__.'/../../..';
include_once $FBCODE_ROOT.'/hphp/tools/command_line_lib.php';

type Result = int;
class TestResult {
  const Result FATAL = -1;
  const Result CLEAN = 0;
  const Result ERRORS = 1;
}

class Tests {

  public static Map $framework_info;

  public static function init() {
    // HERE ARE THE CURRENTLY AVAILABLE TESTS
    self::$framework_info =
      Map {
        'assetic' =>
          Map {
            'install_root' => __DIR__."/frameworks/assetic",
            'test_root' =>  __DIR__."/frameworks/assetic",
            'git_path' => "git@github.com:kriswallsmith/assetic.git",
          },
        'paris' =>
          Map {
            'install_root' => __DIR__."/frameworks/paris",
            'test_root' => __DIR__."/frameworks/paris",
            'git_path' => "git@github.com:j4mie/paris.git",
          },
        'idiorm' =>
          Map {
            'install_root' => __DIR__."/frameworks/idiorm",
            'test_root' => __DIR__."/frameworks/idiorm",
            'git_path' => "git@github.com:j4mie/idiorm.git",
        }
      };
  }
}

function run(OptionInfoMap $options): void {
  if ($options->containsKey('help')) {
    return help();
  }

  $verbose = false;
  if($options->containsKey('verbose')) {
    $verbose = true;
  }

  $timeout = 1800; // 1800 seconds is the default run time before timeout
  if ($options->containsKey('timeout')) {
    $timeout = (int) $options['timeout'];
  }

  // Tests specified at the command line
  if ($options->containsKey('tests')) {
    // Replace multiple spaces with one space
    $tests = preg_replace('!\s+!', ' ', $options['tests']);
    $tests_to_run = Vector::fromArray(str_getcsv($tests, " "));
    run_tests($tests_to_run, $timeout, $verbose);
  } else if ($options->containsKey('all')) {
    // Running all tests
    run_tests(Tests::$framework_info->keys(), $timeout, $verbose);
  }else {
    error("Specify tests to run via --tests or --all");
  }
}

// This function should only be called once for a framework (assuming you don't
// delete the framework from your repo). The proxy could make things a bit
// adventurous, so we will see how this works out after some time to test it
// out
function install_framework(string $name, string $install_root,
                           bool $verbose): void {
  $fbcode_root_dir = __DIR__.'/../../..';
  $frameworks_root_dir = __DIR__."/frameworks";
  if (!(file_exists($frameworks_root_dir))) {
    mkdir($frameworks_root_dir, 0755, true);
  }

  // Get the source from GitHub
  verbose("Retrieving framework $name.....\n", $verbose);
  $git_command = $verbose ? "git clone" : "git clone --quiet";
  $git_command .= " ".Tests::$framework_info[$name]['git_path'];
  $git_ret_val = run_install_via_process($git_command, "",
                                         $frameworks_root_dir);
  if ($git_ret_val !== 0) {
    error("Could not download framework $name!\n");
  }

  // Check to see if composer dependencies are necessary to run the test
  if (file_exists($install_root.'/composer.json')) {
    verbose("Retrieving dependencies for framework $name......\n", $verbose);
    // Used for composer-based dependency installs
    $proxy_information =
      Map {
        "HOME" => getenv("HOME"),
        "http_proxy" => "http://fwdproxy.any.facebook.com:8080",
        "https_proxy" => "http://fwdproxy.any.facebook.com:8080",
        "HTTPS_PROXY" => "http://fwdproxy.any.facebook.com:8080",
        "HTTP_PROXY" => "http://fwdproxy.any.facebook.com:8080",
        "HTTP_PROXY_REQUEST_FULLURI" => "true",
      };
    $framework_dependencies_install = "php ".$fbcode_root_dir.
                                      "/hphp/tools/hhvm_wrapper.php ".__DIR__.
                                      "/composer.phar install --dev";
    $comp_command = $verbose ? $framework_dependencies_install :
                               $framework_dependencies_install." --quiet";
    $comp_ret_val = run_install_via_process($comp_command, "",
                                            $install_root, $proxy_information);
    if ($comp_ret_val !== 0) {
      // Let's just really make sure the dependencies didn't get installed
      // by checking the vendor directories to see if they are empty.
      if (file_exists($install_root."/vendor")) {
        // If there is no content in the directories under vendor, then we
        // did not get the dependencies.
        if (any_dir_empty_one_level($install_root."/vendor")) {
          remove_dir_recursive($install_root);
          error("Couldn't download dependencies for $name!".
                "Removing framework\n");
        }
      } else { // No vendor directory. Dependencies could not have been gotten.
        remove_dir_recursive($install_root);
        error("Couldn't download dependencies for $name!".
              "Removing framework\n");
      }
    }
  }
}

// This will run processes that will get the frameworks and its
// dependencies. Use the default environment if none specified.
function run_install_via_process(string $proc, string $args,
                                 string $path, Map $env= null): ?int
{
  $descriptorspec = array(
    0 => array("pipe", "r"),
    1 => array("pipe", "w"),
    2 => array("pipe", "w"),
  );

  $env_arr = null;
  if ($env !== null) {
    $env_arr = $env->toArray();
  }

  $process = proc_open($proc, $descriptorspec, $pipes, $path, $env_arr);
  if (is_resource($process)) {
    fwrite($pipes[0], $args);
    fclose($pipes[0]);
    $ret = proc_close($process);
    return $ret;
  }
  return null;
}

// Return an int since this will be the main function called
// when running a bunch of tests in process. The int will be
// the code for exit(). Right now we are just returning 0 all
// the time, but when integrate with fbmake tests, then will
// make the return more granular and this function may need to
// be reworked a bit.
function run_single_test(string $name, string $testPath, string $command,
                         string $rawResultsFile, string $summaryFile,
                         int $timeout, bool $verbose): int {

  echo "Running the test for $name\n";
  verbose("Path: $testPath\n", $verbose);
  verbose("Command: $command\n", $verbose);

  $install_root = Tests::$framework_info[$name]['install_root'];
  if (!(file_exists($install_root))) {
    install_framework($name, $install_root, $verbose);
  }
  chdir($testPath);
  // Remember, timeouts are in minutes.
  $run_command = __DIR__."/../../tools/timeout.sh -t $timeout";
  $run_command .= " $command &> $rawResultsFile";
  system($run_command, $retval);
  chdir(__DIR__);
  verbose("Finished running the unit tests......\n", $verbose);
  // FIX: For some tests, HHVM is returning an exit code of 2 (e.g., Slim).
  // Not sure why but I am not going to let that stop us from pushing this for
  // now as it is not a show stopper.
  if ($retval !== 0 && $retval !== 2) {
    verbose("******\n", $verbose);
    verbose("ERROR RUNNING TEST! Could be due to a HHVM fatal or\n", $verbose);
    verbose("timeout, but if not, make sure all test args are set\n", $verbose);
    verbose("correctly. For example, if you have an argument\n", $verbose);
    verbose("value that has spaces, make sure you quote that\n", $verbose);
    verbose("argument. e.g., --arg1 \"my value\"", $verbose);
    verbose("******\n", $verbose);
  }
  verbose("Removing any escape chars that impede results parsing.\n", $verbose);
  remove_escape_characters($rawResultsFile);
  // We may not get to the summary file creation if we timed out or some other
  // horrible fatal.
  $pct_str = create_summary_for_test($name, $rawResultsFile, $summaryFile);
  echo strtoupper($name)." TEST COMPLETE with pass percentage of: $pct_str\n\n";
  echo "Results File: $rawResultsFile\n";
  echo "Summary File: $summaryFile\n";
  echo "..........\n\n";
  return 0;
}

function run_tests_in_processes(Vector $tests_to_run, bool $verbose) {
  $num_processes = min(count($tests_to_run), num_cpus() + 1);

  $children = Vector {};
  for ($i = 0; $i < $num_processes; $i++) {
    $pid = pcntl_fork();
    if ($pid === -1) {
      error('Issues creating threads for tests');
    } else if ($pid) {
      $children[] = $pid;
    } else {
      exit(run_single_test($tests_to_run[$i][0], $tests_to_run[$i][1],
                           $tests_to_run[$i][2], $tests_to_run[$i][3],
                           $tests_to_run[$i][4], $tests_to_run[$i][5],
                           $verbose));
    }
  }

  $process_ret_val = 0;
  foreach($children as $child) {
    pcntl_waitpid($child, $status);
    // For now I am returning 0 from run_single_test.
    // So this will always be 0.
    $process_ret_val |= pcntl_wexitstatus($status);
  }
}

function run_tests(Vector $tests, int $timeout, bool $verbose): void {
  $fbcode_root_dir = __DIR__.'/../../..';
  $test_run_command = "php ".$fbcode_root_dir.
                      "/hphp/tools/hhvm_wrapper.php ".
                      __DIR__."/phpunit.phar";
  $summary_results_file = "/tmp/pop_framework_tests_summary";

  $tests_to_run = Vector {};

  foreach ($tests as $test) {
    $test_name = trim(strtolower($test));
    if (Tests::$framework_info->containsKey($test_name)) {
      $test_path = Tests::$framework_info[$test_name]['test_root'];
      $test_raw_results_file = tempnam("/tmp", $test_name.'-raw-results');
      $tests_to_run[] =
        Vector {
          $test_name,
          $test_path,
          $test_run_command,
          $test_raw_results_file,
          $summary_results_file,
          $timeout
        };
    }
  }

  run_tests_in_processes($tests_to_run, $verbose);

  echo "ALL TESTS COMPLETE!\n";
  echo "SUMMARY:\n";
  $summary = file_get_contents($summary_results_file);
  print $summary;
}

// Sometimes, running PHPUnit adds escape characters if the test doesn't run
// exactly right. Example:
//
// ESC[37;41mESC[2KFAILURES!
// ESC[0mESC[37;41mESC[2KTests: 364, Assertions: 585, Errors: 5.
// ESC[0mESC[2K
//
// Remove the gunk
function remove_escape_characters(string $resultsFile): void {
  $file_data = file_get_contents($resultsFile);
  // Escape characters
  $non_escaped_file_data = preg_replace(
                            '/[\x00-\x08\x0B\x0C\x0E-\x1F\x80-\x9F]/u',
                            '', $file_data);
  // Strange color codes
  $non_escaped_file_data = preg_replace('/\x1b\[[0-9]*m/','', $file_data);
  file_put_contents($resultsFile, $non_escaped_file_data);
}

// We may have to special case frameworks that don't use
// phpunit for their testing (e.g. ThinkUp)
function create_summary_for_test(string $testName,
                                 string $rawResultsFile,
                                 string $summaryFile): string {
  if (($file_contents = file_get_contents($rawResultsFile)) === FALSE) {
    error("Error occured. Probably could not open final ".
          "pass/fail file: $passFailFile");
  }

  // clean pattern represents: OK (364 tests, 590 assertions)
  // error pattern represents: Tests: 364, Assertions: 585, Errors: 5.
  $possible_patterns =
      Map {TestResult::CLEAN => "/OK \(\d+ tests, \d+ assertions\)/",
           TestResult::ERRORS => "/Tests: \d+, Assertions: \d+.*[.]/"};

  $test_result = TestResult::FATAL;
  $match = array();
  foreach ($possible_patterns as $result => $pattern) {
    preg_match($pattern, $file_contents, $match);
    if (!empty($match)) {
      $test_result = $result;
    }
  }

  $pct_str = "";
  switch ($test_result) {
    case TestResult::CLEAN:
      $pct_str = "100";
      break;
    // Get the total number of tests and the number of failures, errors, etc.
    case TestResult::ERRORS:
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
      // Removed skipped tests
      $total_tests_for_calc =
        (float)($parsed_results["Tests"] - $parsed_results["Skipped"]);
      $total_errors_for_calc =
        (float)($parsed_results["Errors"] + $parsed_results["Failures"] +
        $parsed_results["Incomplete"]);
      $pct = ($total_tests_for_calc - $total_errors_for_calc)
            / $total_tests_for_calc;
      $pct = round($pct, 4);
      $pct *= 100; // for percentage
      $pct_str = "$pct";
      break;
    case TestResult::FATAL:
      $pct_str = "Fatal";
      break;
    default:
      $pct_str = "Unknown";
      break;
  }

  // Make sure we are overwriting test results if they exit already,
  // as opposed to having duplicate test names
  $file_data = file($summaryFile);
  if ($file_data === false || $file_data === NULL ) {
    file_put_contents($summaryFile, "$testName=$pct_str".PHP_EOL);
  } else {
    $duplicate_match = preg_grep("/$testName/", $file_data);
    if (!empty($duplicate_match)) {
      $file_data = preg_replace("/$testName=.*".PHP_EOL."/",
                   "$testName=$pct_str".PHP_EOL, $file_data);
    } else {
      $file_data[] = "$testName=$pct_str".PHP_EOL;
    }
    file_put_contents($summaryFile, $file_data);
  }

  return $pct_str;
}

function help(): void {
  $intro = <<<INTRO
  oss_framework_test_script

    This script runs one or more open source (oss) framework unit tests
    via PHPUnit. Run one test via the command line or multiple tests via
    a configuration file passed to the command line. The configuration file
    is of the following form, one framework per line:

      <name of test to be run>

    Comment out any line in a configuration file with # so a test won't run.

INTRO;

  $examples = <<<EXAMPLES
  Usage:

    # Run all framework tests
    % oss_framework_test_script --all

    # Run all framework tests with a timeout per test (in secs)
    % oss_framework_test_script --all --timeout 600

    # Run one test (encompassed by quotes)
    % oss_framework_test_script --tests "Composer"

    # Run multiple tests (space delimited encompassed by quotes)
    % oss_framework_test_script --tests "composer assetic paris"

    # Run multiple tests with timeout for each test (in seconds)
    % oss_framework_test_script --tests "composer assetic" --timeout 600

    # Display help
    % oss_framework_test_script --help

EXAMPLES;

  display_help($intro, oss_test_option_map());
  echo $examples;

}

function oss_test_option_map(): OptionInfoMap {
  return Map {
    'help'                => Pair {'h', "Print help message"},
    'tests:'              => Pair {'', "Tests to be run. Specify space ".
                                       "separated tests after this argument ".
                                       "in quotes. If a test doesn't exist ".
                                       "in the tests Map in this code, it ".
                                       "won't be run."},
    'all'                 => Pair {'', "Run all framework tests. The tests ".
                                       "to be run are hardcoded in a Map in ".
                                       "this code."},
    'timeout:'            => Pair {'t', "Optional - The maximum amount of ".
                                        "time, in secs, to allow a test to ".
                                        "run"},
    'verbose'             => Pair {'v', "Optional - For a lot of messages
                                         about what is going on"},
  };
}

// For determining number of processes
function num_cpus() {
  switch(PHP_OS) {
    case 'Linux':
      $data = file('/proc/stat');
      $cores = 0;
      foreach($data as $line) {
        if (preg_match('/^cpu[0-9]/', $line)) {
          $cores++;
        }
      }
      return $cores;
    case 'Darwin':
    case 'FreeBSD':
      return exec('sysctl -n hw.ncpu');
  }
  return 2; // default when we don't know how to detect
}

function remove_dir_recursive(string $dir) {
  $files = new RecursiveIteratorIterator(
    new RecursiveDirectoryIterator($dir, RecursiveDirectoryIterator::SKIP_DOTS),
    RecursiveIteratorIterator::CHILD_FIRST);

  foreach ($files as $fileinfo) {
    $todo = ($fileinfo->isDir() ? 'rmdir' : 'unlink');
    $todo($fileinfo->getRealPath());
  }
  rmdir($dir);
}

function any_dir_empty_one_level(string $dir): bool {
  $files = scandir($dir);
  // Get rid of any "." and ".." to check
  unset($files[array_search(".",$files)]);
  unset($files[array_search("..",$files)]);
  foreach ($files as $file) {
    if (is_dir($dir."/".$file)) {
      // Empty dir will have . and ..
      if (count(scandir($dir."/".$file)) <= 2) {
        return true;
      }
    }
  }
  return false;
}

function verbose(string $msg, bool $verbose): void {
  if ($verbose) {
    echo $msg;
  }
}

function main(): void {
  echo "Script running....Be patient as some tests take a while with a debug ".
       "build of HHVM\n";
  $options = parse_options(oss_test_option_map());
  Tests::init();
  run($options);
  echo "Script finished!!!\n";
}

main();
