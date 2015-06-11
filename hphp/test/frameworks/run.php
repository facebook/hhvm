<?hh
/*
 * This script allows us to more easily test the key OSS PHP frameworks
 * that is helping bring HHVM closer to parity.
 *
 * Key features:
 *
 *   - Autodownload of frameworks, so we don't have to add 3 GB of frameworks to
 *     our official repo. This can be a bit flaky due to our proxy; so we
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
 *   - Integrate the testing with our current "test/run" style infrastructure
 *     for official pass/fail statistics when we have diffs.
 *
 *   - Special case frameworks that don't use PHPUnit (e.g. Thinkup).
 *
 *   - Decide whether proxy-based code needs to be solidified better or whether
 *     we should just add the frameworks to our repo officially.
 *
 *
*/

require_once __DIR__.'/../../../hphp/tools/command_line_lib.php';
require_once __DIR__.'/utils.php';
require_once __DIR__.'/Colors.php';
require_once __DIR__.'/ProxyInformation.php';
require_once __DIR__.'/PHPUnitPatterns.php';
require_once __DIR__.'/Framework.php';
require_once __DIR__.'/Runner.php';
require_once __DIR__.'/Options.php';
require_once __DIR__.'/spyc/Spyc.php';

function prepare(Set $available_frameworks, Set $framework_class_overrides,
                 Vector $passed_frameworks): Vector {
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
    // Run all the frameworks, except the ones we listed.
    $passed_frameworks  = Vector::fromItems(array_diff(
                                            $available_frameworks->toVector(),
                                            $passed_frameworks));
  } else if ($passed_frameworks->isEmpty()) {
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
      if ($framework_class_overrides->contains($name)) {
        $frameworks[] = new $uname($name);
      } else {
        $frameworks[] = new Framework($name);
      }
    }
  }

  if ($frameworks->isEmpty()) {
    error_and_exit(usage());
  }

  return $frameworks;
}

function fork_buckets(Traversable $data, (function(array):int) $callback): int {
  $num_threads = min(count($data), num_cpus() + 1);
  if (Options::$num_threads !== -1) {
    $num_threads = min(count($data), Options::$num_threads);
  }

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

function list_tests(Vector $frameworks): void {
  if ($frameworks->isEmpty()) {
    error_and_exit("No frameworks available on which to list tests");
  }

  foreach($frameworks as $framework) {
    foreach($framework->getTests() as $test) {
      print fbmake_test_name($framework, $test).PHP_EOL;
    }
  }
}

function run_tests(Vector $frameworks): void {
  if ($frameworks->isEmpty()) {
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
                                        PHPUnitPatterns::STATUS_CODE_PATTERN,
                                        PHPUnitPatterns::STOP_PARSING_PATTERN);
      human(Colors::YELLOW.$framework->getName().Colors::NONE.": running. ".
            "Comparing against ".count($framework->getCurrentTestStatuses()).
            " tests\n");
      $run_msg = "Comparing test suite with previous run. ".
        Colors::GREEN."Green . (dot) ".Colors::NONE.
        "means test result same as previous. ".
        "A ".Colors::GREEN." green F ".Colors::NONE.
        "means we have gone from fail to pass. ".
        "A ".Colors::RED." red F ".Colors::NONE.
        "means we have gone from pass to fail. ".
        "A ".Colors::BLUE." blue F ".Colors::NONE.
        "means we have gone from one type of fail to another type ".
        "of fail (e.g., F to E, or I to F). ".
        "A ".Colors::LIGHTBLUE." light blue F ".Colors::NONE.
        "means we are having trouble accessing the tests from the ".
        "expected run and can't get a proper status".PHP_EOL;
      verbose($run_msg);
    } else {
      human("Establishing baseline statuses for ".$framework->getName().
            " with gray dots...\n");
    }

    // If we are running the tests for the framework in parallel, then let's
    // get all the test for that framework and add each to our tests
    // vector; otherwise, we are just going to add the framework to run
    // serially and use its global phpunit test run command to run the entire
    // suite (just like a normal phpunit run outside this framework).
    $filter_tests = Options::$filter_tests;
    if ($filter_tests !== null) {
      foreach($framework->getTests() as $test) {
        $testname = fbmake_test_name($framework, $test);
        if ($filter_tests->contains($testname)) {
          $st = new Runner($framework, $test);
          $all_tests->add($st);
        }
      }
    } elseif ($framework->isParallel() && !Options::$as_phpunit) {
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
  human("Beginning the unit tests.....\n");
  if ($all_tests->isEmpty()) {
    error_and_exit("No tests found to run");
  }

  fork_buckets($all_tests, fun('run_test_bucket'));

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
    $msg = <<<THUMBSUP
All tests ran as expected.


          _
         /(|
        (  :
       __\  \  _____
     (____)  -|
    (____)|   |
     (____).__|
      (___)__.|_____

THUMBSUP;
   human($msg);
  } else {
    $msg = <<<THUMBSDOWN
All tests did not run as expected. Either some statuses were
different or the number of tests run didn't match the number of
tests expected to run


      ______
     (( ____ \-
     (( _____
     ((_____
     ((____   ----
          /  /
         (_((

THUMBSDOWN;
    human($msg);
  }

  // Print the diffs
  if (
    (Options::$output_format === OutputFormat::HUMAN) ||
    (Options::$output_format === OutputFormat::HUMAN_VERBOSE)
  ) {
    foreach($diff_frameworks as $framework) {
      print_diffs($framework);
    }
  }

  // Print out summary information
  print_summary_information($summary_file);

  // Update any git hashes in case --latest or --latest-record was used and we
  // changed the hashes currently in frameworks.json. Use md5 of the original
  // and current maps to see if we are different
  if (md5(serialize(Options::$original_framework_info)) !==
      md5(serialize(Options::$framework_info))) {
    human("Updating frameworks.json because some hashes have been updated");
    file_put_contents(__DIR__."/frameworks.yaml",
                      Spyc::YAMLDump(Options::$framework_info));
  }
}

function run_test_bucket(array $test_bucket): int {
  $result = 0;
  foreach($test_bucket as $test) {
    $result = $test->run();
  }
  return $result;
}

function get_unit_testing_infra_dependencies(): void {
  // Even if we have vendor/, try and grab composer in case we're missing
  // we're running a framework that isn't already installed.
  if (
    (!Options::$local_source_only) &&
    (!(file_exists(__DIR__."/composer.phar")) ||
      (time() - filectime(__DIR__."/composer.phar")) >= 29*24*60*60)
  ) {
    human("Getting composer.phar....\n");
    delete_file(__DIR__."/composer.phar");
    $comp_url = "http://getcomposer.org/composer.phar";
    $get_composer_command = "curl ".$comp_url." -o ".
      __DIR__."/composer.phar 2>&1";
    $ret = run_install($get_composer_command, __DIR__,
                       ProxyInformation::$proxies);
    if ($ret !== 0) {
      error_and_exit("Could not download composer. Script stopping\n");
    }
  }

  $checksum = md5(serialize([
    // Use both in case composer.json has been changed, but the lock file
    // hasn't been updated yet.
    file_get_contents(__DIR__.'/composer.json'),
    file_get_contents(__DIR__.'/composer.lock'),
  ]));
  $stamp_file = __DIR__.'/vendor/'.$checksum.'.stamp';
  if (file_exists($stamp_file)) {
    return;
  }

  $cache_dir = Options::$cache_directory;
  $cache_file = $cache_dir.'/vendor-'.$checksum.'.tar.bz2';
  if (!file_exists($stamp_file)) {
    invariant(
      file_exists($cache_file) || !Options::$local_source_only,
      '--local-source-only specified, but no vendor cache'
    );
    if ($cache_dir && file_exists($cache_file)) {
      human("Extracting vendor cache, eg PHPUnit...\n");
      $pd = new PharData($cache_file);
      $pd->extractTo(__DIR__);
      invariant(file_exists($stamp_file), 'Failed to extract vendor cache');
      invariant(
        file_exists(__DIR__.'/vendor/bin/phpunit'),
        'PHPUnit executable not in vendor cache',
      );
      return;
    }
  }

  // We don't have a cached vendor/, but as --local-source-only wasn't
  // specified, we can try to download it.

  // Quick hack to make sure we get the latest phpunit binary from composer
  $md5_file = __DIR__."/composer.json.md5";
  $vendor_dir = __DIR__."/vendor";
  $lock_file = __DIR__."/composer.lock";
  if (!file_exists($md5_file) ||
      file_get_contents($md5_file) !== $checksum) {
    human("\nUpdated composer.json found. Updating phpunit binary.\n");
    if (file_exists($vendor_dir)) {
      remove_dir_recursive($vendor_dir);
    }
    delete_file($lock_file);
    file_put_contents($md5_file, $checksum);
  }

  // Install phpunit from composer.json located in __DIR__
  $phpunit_binary = __DIR__."/vendor/bin/phpunit";
  if (!(file_exists($phpunit_binary))) {
    human("\nDownloading PHPUnit in order to run tests. There may be an ".
          "output delay while the download begins.\n");
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

  touch($stamp_file);
  if ($cache_dir) {
    run_install(
      'tar jcf '.escapeshellarg($cache_file).' vendor/',
      __DIR__
    );
  }
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
    $decoded_results = json_decode($contents, true);
    ksort($decoded_results);

    switch (Options::$output_format) {
      case OutputFormat::CSV:
        if (Options::$csv_header) {
          $print_str = str_pad("date,", 20);
          foreach ($decoded_results as $key => $value) {
            $print_str .= str_pad($key.",", 20);
          }
          print rtrim($print_str, " ,") . PHP_EOL;
        }
        $print_str = str_pad(date("Y/m/d-G:i:s").",", 20);
        foreach ($decoded_results as $key => $value) {
          $print_str .= str_pad($value.",", 20);
        }
        print rtrim($print_str, " ,") . PHP_EOL;
        break;
      case OutputFormat::FBMAKE:
        break;
      default:
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
        break;
    }
  } else {
      human("\nNO SUMMARY INFO AVAILABLE!\n");
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

    # Run all framework tests using another PHP binary
    % hhvm run.php --all --with-php ~/php55/bin/php

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

    # Run tests with the runner as they would be run with phpunit
    % hhvm run.php --as-phpunit paris
    % hhvm run.php --as-phpunit --all (THIS WILL BE SLOW AND COULD FATAL TOO)

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

function get_available_frameworks(): array<string> {
  return array_keys(Spyc::YAMLLoad(__DIR__.'/frameworks.yaml'));
}

function usage(): string {
  $msg = "Specify frameworks to run, use --all or use --allexcept. ";
  $msg .= "Available frameworks are: ".PHP_EOL;
  $msg .= implode(PHP_EOL, get_available_frameworks());
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
    'flakey'              => Pair {'',  'Include tests that intermittently '.
                                        'fail.'},
    'timeout:'            => Pair {'',  "The maximum amount of time, in secs, ".
                                        "to allow a individual test to run.".
                                        "Default is 60 seconds."},
    'verbose'             => Pair {'v', "For a lot of messages about what is ".
                                        "going on."},
    'with-php:'           => Pair {'',  "Use php to run the tests ".
                                        "Currently, php must be installed ".
                                        "and the path to the php binary".
                                        "specified."},
    'install-only'        => Pair {'',  "Download and install the framework, ".
                                        "but don't run any tests."},
    'redownload'          => Pair {'',  "Forces a redownload of the framework ".
                                        "code and dependencies. This uses ".
                                        "the current git hash associated with ".
                                        "the current download."},
    'record'              => Pair {'',  "Forces a new expect file for the ".
                                        "framework test suite"},
    'latest-record'       => Pair {'',  "Forces a complete update of a ".
                                        "framework. The code is updated with ".
                                        "the latest and greatest hash of the ".
                                        "current branch (e.g., master) and ".
                                        "the expect file is updated."},
    'latest'              => Pair {'',  "Forces framework code to be updated ".
                                        "with the latest and greatest hash of ".
                                        "the current branch (e.g., master)."},
    'csv'                 => Pair {'',  "Just create the machine readable ".
                                        "summary CSV for parsing and chart ".
                                        "display."},
    'csvheader'           => Pair {'',  "Add a header line for the summary ".
                                        "CSV which includes the framework ".
                                        "names."},
    'fbmake'              => Pair {'',  "Output a stream of JSON objects that ".
                                        "Facebook's test systems understand"},
    'by-file'             => Pair {'f',  "DEFAULT: Run tests for a framework ".
                                         "on a per test file basis, as ".
                                         "opposed to a an individual test ".
                                         "basis."},
    'by-single-test'      => Pair {'s',  "Run tests for a framework on a ".
                                         "individual test basis, as ".
                                         "opposed to a test file basis. This ".
                                         "basically means the --filter option ".
                                         "is being used for phpunit"},
    'as-phpunit'          => Pair {'',   "Run tests for a framework just ".
                                         "like it would be run normally with ".
                                         "PHPUnit"},
    'numthreads:'         => Pair {'',   "The exact number of threads to use ".
                                         "when running framework tests in ".
                                         "parallel"},
    'isolate'             => Pair {'',   "Try to make tests that have ".
                                         "external dependencies automatically ".
                                         "fail"},
    'cache-directory:'    => Pair {'',   'Directory to store source tarballs'},
    'local-source-only'   => Pair {'',   'Fail if git or composer calls are '.
                                         'needed'},
    'list-tests'          => Pair {'',   'List tests that would be run'},
    'run-specified:'      => Pair {'',   'Run only the specified tests by '.
                                         'name, separated by a comma. Test '.
                                         'names are returned by '.
                                         '--list-tests. If the name is '.
                                         'prepended with an @, load the '.
                                         'test names from file instead.'},
  };
}

function main(array &$argv): void {
  $options = parse_options(oss_test_option_map());
  if ($options->containsKey('help')) {
    help();
    return;
  }

  // Parse all the options passed to run.php
  Options::parse($options);
  $passed_frameworks = new Vector($argv);
  $available_frameworks = new Set(array_keys(Options::$framework_info));
  include_all_php(__DIR__."/framework_class_overrides");
  $framework_class_overrides = get_subclasses_of("Framework")->toSet();
  $frameworks = prepare($available_frameworks, $framework_class_overrides,
                        $passed_frameworks);

  if (Options::$list_tests) {
    list_tests($frameworks);
  } elseif (Options::$run_tests) {
    run_tests($frameworks);
  }
}

main($argv);
