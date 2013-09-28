#!/usr/local/bin/php
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
 *   - Raw PHPUnit results for all tests run during a given execution of the
 *     script are appnended to one big file for easier viewing.
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
 *
*/
$FBCODE_ROOT = __DIR__.'/../../..';
include_once $FBCODE_ROOT.'/hphp/tools/command_line_lib.php';

class SortedIterator extends SplHeap {
  public function __construct(Iterator $iterator) {
    foreach ($iterator as $item) {
      $this->insert($item);
    }
  }

  /* While the parameters are of type mixed, going to assume
     they are SplFileInfos for now. This will sort based on
     path, not including any appended file. Results will be
     similar to:

     /tmp/root/b34.txt
     /tmp/root/a.txt
     /tmp/root/z.txt
     /tmp/root/foo/we.txt
     /tmp/root/foo/a.txt
     /tmp/root/waz/bing.txt

     where the ending files are not necessarily in order, but the containing
     directories are in order.
  */
  public function compare(mixed $b, mixed $a): int {
    return strcmp($a->getPath(), $b->getPath());
  }
}

type Result = int;
class TestResult {
  const Result FATAL = -1;
  const Result CLEAN = 0;
  const Result ERRORS = 1;
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
        };
        return true;
    } else {
        self::$proxies = Map { "HOME" => getenv("HOME") };
        return false;
    }
  }
}

class Tests {

  public static Map $framework_info = null;

  public static function init(): void {
    // HERE ARE THE CURRENTLY AVAILABLE TESTS.
    // Some are commented out because they may need to be special cased as
    // they require special testing commands, mock databases, etc.
    self::$framework_info =
      Map {
        'assetic' =>
          Map {
            'install_root' => __DIR__."/frameworks/assetic",
            'git_path' => "git@github.com:kriswallsmith/assetic.git",
          },
        'paris' =>
          Map {
            'install_root' => __DIR__."/frameworks/paris",
            'git_path' => "git@github.com:j4mie/paris.git",
          },
        'idiorm' =>
          Map {
            'install_root' => __DIR__."/frameworks/idiorm",
            'git_path' => "git@github.com:j4mie/idiorm.git",
          },
        /* Cannot download 1 out of 10 or so symfony dependencies with hhvm.
           Most of them are retrieved except this one:

           - Installing phing/phing (2.6.1)
             Downloading: 100%
             Download failed, retrying...
             Downloading: 100%
             Download failed, retrying...
             Downloading: 100%
             oss_framework_test_script.php: Couldn't download dependencies for
             symfony!Removing framework

           This one works with Zend for some reason?*/

        'symfony' =>
          Map {
            'install_root' => __DIR__."/frameworks/symfony",
            'git_path' => "git@github.com:symfony/symfony.git",
          },
        'codeigniter' =>
          Map {
            'install_root' => __DIR__."/frameworks/CodeIgniter",
            'git_path' => "git@github.com:EllisLab/CodeIgniter.git",
          },
        'laravel' =>
          Map {
            'install_root' => __DIR__."/frameworks/laravel",
            'git_path' => "git@github.com:laravel/framework.git",
          },
        'zf2' =>
          Map {
            'install_root' => __DIR__."/frameworks/zf2",
            'git_path' => "git@github.com:zendframework/zf2.git",
          },
        'yii' =>
          Map {
            'install_root' => __DIR__."/frameworks/yii",
            'git_path' => "git@github.com:yiisoft/yii.git",
            'test_path' => __DIR__."/frameworks/yii/tests",
            'test_run_command' => get_hhvm_build()." ".__DIR__.
                                  "/vendor/bin/phpunit --debug .",
          },
        'slim' =>
          Map {
            'install_root' => __DIR__."/frameworks/Slim",
            'git_path' => "git@github.com:codeguy/Slim.git",
          },
        'wordpress' =>
          Map {
            'install_root' => __DIR__."/frameworks/wordpress-unit-tests",
            'git_path' => "git@github.com:kurtpayne/wordpress-unit-tests.git",
          },
        'composer' =>
          Map {
            'install_root' => __DIR__."/frameworks/composer",
            'git_path' => "git@github.com:composer/composer.git",
          },
        'doctrine2' =>
          Map {
            'install_root' => __DIR__."/frameworks/doctrine2",
            'git_path' => "git@github.com:doctrine/doctrine2.git",
          },
        'twig' =>
          Map {
            'install_root' => __DIR__."/frameworks/Twig",
            'git_path' => "git@github.com:fabpot/Twig.git",
          },
        'joomla' =>
          Map {
            'install_root' => __DIR__."/frameworks/joomla-framework",
            'git_path' => "git@github.com:joomla/joomla-framework.git",
          },
        /* Requires some sort of database setup.
        'magento2' =>
          Map {
            'install_root' => __DIR__."/frameworks/magento2",
            'git_path' => "git@github.com:magento/magento2.git",
          },*/
        'phpmyadmin' =>
          Map {
            'install_root' => __DIR__."/frameworks/phpmyadmin",
            'git_path' => "git@github.com:phpmyadmin/phpmyadmin.git",
          },
        'phpbb3' =>
          Map {
            'install_root' => __DIR__."/frameworks/phpbb3",
            'git_path' => "git@github.com:phpbb/phpbb3.git",
          },
        'pear' =>
          Map {
            'install_root' => __DIR__."/frameworks/pear-core",
            'git_path' => "git@github.com:pear/pear-core.git",
            'test_path' => __DIR__."/frameworks/pear-core",
            'test_run_command' => get_hhvm_build()." ".__DIR__.
                                  "/vendor/bin/phpunit --debug tests",
          },
        /* Requires a mediawiki install to run tests :(
        'mediawiki' =>
          Map {
            'install_root' => __DIR__."/frameworks/mediawiki-core",
            'git_path' => "git@github.com:wikimedia/mediawiki-core.git",
          },*/
        'typo3' =>
          Map {
            'install_root' => __DIR__."/frameworks/typo3",
            'git_path' => "git@github.com:TYPO3/TYPO3.CMS.git",
            'test_path' => __DIR__."/frameworks/typo3",
            'test_run_command' => get_hhvm_build()." cli_dispatch.phpsh ".
                                  __DIR__."/vendor/bin/phpunit --debug",
          },
        'drupal' =>
          Map {
            'install_root' => __DIR__."/frameworks/drupal",
            'git_path' => "git@github.com:drupal/drupal.git",
            // For when we want something other than the deafuly
            'git_branch' => "8.x",
          },
        /*
        'twitteroauth' =>
          Map {
            'install_root' => __DIR__."/frameworks/twitteroauth",
            'git_path' => "git@github.com:abraham/twitteroauth.git",
          },*/
        'thinkup' =>
          Map {
            'install_root' => __DIR__."/frameworks/thinkup",
            'git_path' => "git@github.com:ginatrapani/ThinkUp.git",
            'test_path' => __DIR__."/frameworks/thinkup/tests",
            'test_run_command' => get_hhvm_build()." all_tests.php",
          },
        'cakephp' =>
          Map {
            'install_root' => __DIR__."/frameworks/cakephp",
            'git_path' => "git@github.com:cakephp/cakephp.git",
            'test_path' => __DIR__."/frameworks/cakephp",
            // FIX: May have to update "cake" script to call "hhvm"
            'test_run_command' => "lib/Cake/Console/cake test core AllTests",
          },
        'facebook-php-sdk' =>
          Map {
            'install_root' => __DIR__."/frameworks/facebook-php-sdk",
            'git_path' => "git@github.com:facebook/facebook-php-sdk.git",
            'test_path' => __DIR__."/frameworks/facebook-php-sdk",
            'test_run_command' => get_hhvm_build()." ".__DIR__.
                                  "/vendor/bin/phpunit --debug --stderr ".
                                  "--bootstrap tests/bootstrap.php ".
                                  "tests/tests.php",
          },
        'phpunit' =>
          Map {
            'install_root' => __DIR__."/frameworks/phpunit",
            'git_path' => "git@github.com:sebastianbergmann/phpunit.git",
            'test_path' => __DIR__."/frameworks/phpunit",
            'test_run_command' => get_hhvm_build()." ".__DIR__.
                                  "/frameworks/phpunit/phpunit.php --debug",
          },
      };
  }
}

function run(OptionInfoMap $options, Vector $tests): void {
  // HACK: Yes, this next bit of "removeKey" code is hacky, maybe even clowny.
  // We can fix the command_line_lib.php to maybe make things a bit better.

  // It is possible that the $tests vector came in here with a combiniation
  // of command line options (e.g., verbose and timeout) that should be
  // removed before running the tests. Remeber all these option values are
  // already set in $options. They are just artificats of $argv right now.
  // Although, there is a failsafe when checking if the test exists that would
  // weed command line opts out too.
  $verbose = false;
  if($options->containsKey('verbose')) {
    $verbose = true;
    // $tests[0] may not even be "verbose", but it doesn't matter, we are
    // just trying to make the count right for $tests
    $tests->removeKey(0);
  }

  $timeout = 1800; // 1800 seconds is the default run time before timeout
  if ($options->containsKey('timeout')) {
    $timeout = (int) $options['timeout'];
    // Remove timeout option and its value from the $tests vector
    $tests->removeKey(0);
    $tests->removeKey(0);
  }

  $zend_path = null;
  if ($options->containsKey('zend')) {
    echo "Will try Zend if necessary. If Zend doesn't work, the script will ".
         "still continue running; the particular framework on which Zend was ".
         "attempted may not be available though.\n";
    $zend_path = $options['zend'];
    $tests->removeKey(0);
    $tests->removeKey(0);
  }

  $force_redownload = false;
  if ($options->containsKey('redownload')) {
    $force_redownload = true;
    $tests->removeKey(0);
  }

  if ($options->containsKey('all')) {
    $tests->removeKey(0);
    // At this point, $tests should be empty if we are in --all mode.
    if (!($tests->isEmpty())) {
      error("Do not specify both --all and tests to run at the same time.\n");
    }
    // Running all tests
    run_tests(Tests::$framework_info->keys(), $timeout, $verbose,
              $force_redownload, $zend_path);
  } else if (count($tests) > 0) {
    // Tests specified at the command line. At this point, $tests should only
    // have tests to be run
    run_tests($tests, $timeout, $verbose, $force_redownload, $zend_path);
  } else {
    error("Specify tests to run or use --all");
  }
}

// This function should only be called once for a framework (assuming you don't
// delete the framework from your repo). The proxy could make things a bit
// adventurous, so we will see how this works out after some time to test it
// out
function install_framework(string $name, string $install_root,
                           bool $verbose, ?string $zend_path): void {
  $frameworks_root_dir = __DIR__."/frameworks";
  if (!(file_exists($frameworks_root_dir))) {
    mkdir($frameworks_root_dir, 0755, true);
  }

  // Get the source from GitHub
  verbose("Retrieving framework $name.....\n", $verbose);
  $git_command = $verbose ? "git clone" : "git clone --quiet";
  if (Tests::$framework_info[$name]->contains('git_branch')) {
    $git_command .= " -b ".Tests::$framework_info[$name]['git_branch'];
  }
  $git_command .= " ".Tests::$framework_info[$name]['git_path'];
  $git_command .= " ".Tests::$framework_info[$name]['install_root'];
  $git_ret = run_install_via_process($git_command, "",
                                     $frameworks_root_dir, null, $verbose);
  if ($git_ret !== 0) {
    error("Could not download framework $name!\n");
  }

  $composer_json_path = find_any_file_recursive(Set {"composer.json"},
                                                $install_root, true);
  verbose("composer.json found in: $composer_json_path\n", $verbose);
  // Check to see if composer dependencies are necessary to run the test
  if ($composer_json_path !== null) {
    verbose("Retrieving dependencies for framework $name......\n", $verbose);
    $framework_dependencies_install = get_hhvm_build()." ".__DIR__.
                                      "/composer.phar install --dev";
    $framework_dependencies_install .= $verbose ? " --verbose" : " --quiet";
    $install_ret = run_install_via_process($framework_dependencies_install, "",
                                           $composer_json_path,
                                           ProxyInformation::$proxies,
                                           $verbose);
    // If provided the option at the command line, try Zend if we are not
    // successful with hhvm. For example, I know hhvm had trouble
    // downloading dependencies for Symfony, but Zend worked.
    //
    // FIX: Should we try to install a vanilla zend binary here instead of
    // relying on user to specify a path? Should we try to determine if zend
    // is already installed via a $PATH variable?
    if ($zend_path !== null && $install_ret !== 0) {
      echo "HHVM didn't work for downloading dependencies for $name. ".
           "Trying Zend.\n";
      $framework_dependencies_install = $zend_path." ".__DIR__.
                                        "/composer.phar install --dev";
      $framework_dependencies_install .= $verbose ? " --verbose" : " --quiet";
      $install_ret = run_install_via_process($framework_dependencies_install,
                                             "", $composer_json_path,
                                             ProxyInformation::$proxies,
                                             $verbose);
    }

    if ($install_ret !== 0) {
      // Let's just really make sure the dependencies didn't get installed
      // by checking the vendor directories to see if they are empty.
      $fw_vendor_dir = find_any_file_recursive(Set {"vendor"}, $install_root,
                                               false);
      if ($fw_vendor_dir !== null) {
        // If there is no content in the directories under vendor, then we
        // did not get the dependencies.
        if (any_dir_empty_one_level($fw_vendor_dir)) {
          remove_dir_recursive($install_root);
          error("Couldn't download dependencies for $name!".
                " Removing framework\n");
        }
      } else { // No vendor directory. Dependencies could not have been gotten.
        remove_dir_recursive($install_root);
        error("Couldn't download dependencies for $name!".
              " Removing framework\n");
      }
    }
  }
}

// This will run processes that will get the frameworks and its
// dependencies. Use the default environment if none specified.
function run_install_via_process(string $proc, string $args,
                                 string $path, ?Map $env,
                                 bool $verbose): ?int
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

  $pipes = null;
  $process = proc_open($proc, $descriptorspec, $pipes, $path, $env_arr);
  if (is_resource($process)) {
    fwrite($pipes[0], $args);
    fclose($pipes[0]);
    if ($verbose) {
      echo stream_get_contents($pipes[1]);
      fclose($pipes[1]);
    }
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
function run_single_test(string $test_name, string $summary_results_file,
                         string $appended_raw_results_file, int $timeout,
                         bool $verbose, bool $force_redownload,
                         ?string $zend_path): int {
  $fbcode_root_dir = __DIR__.'/../../..';

  $test_raw_results_file = tempnam("/tmp", $test_name.'-raw-results');

  // Remember, timeouts are in minutes.
  $run_command = __DIR__."/../../tools/timeout.sh -t $timeout";
  if (Tests::$framework_info[$test_name]->contains('test_run_command')) {
    $run_command .= " ".Tests::$framework_info[$test_name]['test_run_command'];
  } else {
    $run_command .= " ".get_hhvm_build()." ".__DIR__.
                    "/vendor/bin/phpunit --debug";
  }
  $run_command .= " &> $test_raw_results_file";

  // 2 possibilities, phpunit.xml and phpunit.xml.dist for configuration
  $phpunit_config_files = Set {'phpunit.xml', 'phpunit.xml.dist'};

  echo "Running test for $test_name in output file $test_raw_results_file\n";
  verbose("Command: $run_command\n", $verbose);

  $install_root = Tests::$framework_info[$test_name]['install_root'];

  if (!(file_exists($install_root))) {
    install_framework($test_name, $install_root, $verbose, $zend_path);
  } else if ($force_redownload) {
    remove_dir_recursive($install_root);
    install_framework($test_name, $install_root, $verbose, $zend_path);
  }

  $test_path = null;
  if (Tests::$framework_info[$test_name]->contains('test_path')) {
    $test_path = Tests::$framework_info[$test_name]['test_path'];
  } else {
    $phpunit_config_file_loc = null;
    $phpunit_config_file_loc = find_any_file_recursive($phpunit_config_files,
                                                       $install_root, true);
    // The test path will be where the phpunit config file is located since
    // that file contains the test directory name. If no config file, error.
    $test_path = $phpunit_config_file_loc !== null
               ? $phpunit_config_file_loc
               : error("No phpunit test directory found for $test_name");
    verbose("Using phpunit xml file in: $phpunit_config_file_loc\n", $verbose);
  }

  verbose("Test Path: $test_path\n", $verbose);
  chdir($test_path);
  $retval = -1;
  system($run_command, $retval);
  chdir(__DIR__);
  verbose("Finished running the unit tests......\n", $verbose);
  // FIX: For some tests, HHVM is returning an exit code of 2 (e.g., Slim).
  // Not sure why but I am not going to let that stop us from pushing this for
  // now as it is not a show stopper.
  if ($retval !== 0 && $retval !== 2) {
    verbose("******\n", $verbose);
    verbose("ERROR RUNNING TEST! Could be due to a HHVM fatal or\n", $verbose);
    verbose("timeout, but if not, make sure all tests are\n", $verbose);
    verbose("spelled correctly at the command line.\n", $verbose);
    verbose("******\n", $verbose);
  }
  verbose("Removing any escape chars that impede results parsing.\n", $verbose);
  remove_escape_characters($test_raw_results_file);
  // We may not get to the summary file creation if we timed out or some other
  // horrible fatal.
  $pct_str = create_summary_for_test($test_name, $test_raw_results_file,
                                     $summary_results_file,
                                     $appended_raw_results_file);
  echo strtoupper($test_name).
       " TEST COMPLETE with pass percentage of: $pct_str\n\n";
  echo "Results File: $test_raw_results_file\n";
  echo "Summary File: $summary_results_file\n";
  echo "..........\n\n";
  return 0;
}

function run_tests_in_processes(Vector $tests_to_run,
                                string $summary_results_file,
                                string $appended_raw_results_file,
                                int $timeout, bool $verbose,
                                bool $force_redownload,
                                ?string $zend_path): void {
  $num_processes = min(count($tests_to_run), num_cpus() + 1);

  $children = Vector {};
  for ($i = 0; $i < $num_processes; $i++) {
    $pid = pcntl_fork();
    if ($pid === -1) {
      error('Issues creating threads for tests');
    } else if ($pid) {
      $children[] = $pid;
    } else {
      exit(run_single_test($tests_to_run[$i], $summary_results_file,
                           $appended_raw_results_file, $timeout, $verbose,
                           $force_redownload, $zend_path));
    }
  }

  $process_ret_val = 0;
  $status = -1;
  foreach($children as $child) {
    pcntl_waitpid($child, $status);
    // For now I am returning 0 from run_single_test.
    // So this will always be 0.
    $process_ret_val |= pcntl_wexitstatus($status);
  }
}

function run_tests(Vector $tests, int $timeout,
                  bool $verbose, bool $force_redownload,
                  ?string $zend_path): void {
  // Install composer.phar
  if (!(file_exists(__DIR__."/composer.phar"))) {
    verbose("Getting composer.phar....\n", $verbose);
    $comp_url = "http://getcomposer.org/composer.phar";
    $get_composer_command = $verbose
                            ? "wget --verbose"
                            : "wget --quiet";
    $get_composer_command .= " ".$comp_url." -P ".__DIR__;
    system($get_composer_command, $ret);
    if ($ret !== 0) {
      error("Could not download composer. Script stopping\n");
    }
  }

  // Install phpunit from composer.json located in __DIR__
  $phpunit_binary = __DIR__."/vendor/bin/phpunit";
  if (!(file_exists($phpunit_binary))) {
    verbose("Donwloading PHPUnit in order to run tests...\n", $verbose);
    $phpunit_install_command = get_hhvm_build()." ".__DIR__.
                               "/composer.phar install --dev";
    $phpunit_install_command .= $verbose ? " --verbose" : " --quiet";
    run_install_via_process($phpunit_install_command, "", __DIR__,
                            ProxyInformation::$proxies, $verbose);
    system($phpunit_install_command, $ret);
    if ($ret !== 0) {
      error("Could not install PHPUnit. Script stopping\n");
    }
  }


  $summary_results_file = "/tmp/pop_framework_tests_summary";
  $appended_raw_results_file = tempnam("/tmp", 'appended-raw-results');
  $tests_to_run = Vector {};
  foreach ($tests as $test) {
    $test_name = trim(strtolower($test));
    if (Tests::$framework_info->containsKey($test_name)) {
      $tests_to_run[] = $test_name;
    }
  }

  if (count($tests_to_run) > 0) {
    run_tests_in_processes($tests_to_run, $summary_results_file,
                           $appended_raw_results_file, $timeout, $verbose,
                           $force_redownload, $zend_path);
    echo "ALL TESTS COMPLETE!\n";
    echo "ALL TEST RESULTS FOUND IN: ".$appended_raw_results_file."\n";
    echo "SUMMARY:\n";
    $summary = file_get_contents($summary_results_file);
    print $summary;
  } else {
    error("No valid tests to run. Try different tests or the --all option");
  }
}

// Sometimes, running PHPUnit adds escape characters if the test doesn't run
// exactly right. Example:
//
// ESC[37;41mESC[2KFAILURES!
// ESC[0mESC[37;41mESC[2KTests: 364, Assertions: 585, Errors: 5.
// ESC[0mESC[2K
//
// Remove the gunk
function remove_escape_characters(string $results_file): void {
  $file_data = file_get_contents($results_file);
  // Escape characters
  $non_escaped_file_data = preg_replace(
                            '/[\x00-\x08\x0B\x0C\x0E-\x1F\x80-\x9F]/u',
                            '', $file_data);
  // Strange color codes
  $non_escaped_file_data = preg_replace('/\x1b\[[0-9]*m/','', $file_data);
  file_put_contents($results_file, $non_escaped_file_data);
}

// We may have to special case frameworks that don't use
// phpunit for their testing (e.g. ThinkUp)
function create_summary_for_test(string $test_name,
                                 string $raw_results_file,
                                 string $summary_file,
                                 string $appended_raw_results_file): string {
  // Get the last 4 lines of the results file which will give us the surface
  // area necessary to get the final stats.
  $file = escapeshellarg($raw_results_file);
  if (($lines = `tail -n 4 $file`) === "") {
    error("Error occured. Probably could not open final ".
          "pass/fail file: $raw_results_file");
  }

  // clean pattern represents: OK (364 tests, 590 assertions)
  // error pattern represents: Tests: 364, Assertions: 585, Errors: 5.
  $possible_patterns =
      Map {TestResult::CLEAN => "/OK \(\d+ tests, \d+ assertions\)/",
           TestResult::ERRORS => "/Tests: \d+, Assertions: \d+.*[.]/"};

  $test_result = TestResult::FATAL;
  $match = array();
  foreach ($possible_patterns as $result => $pattern) {
    preg_match($pattern, $lines, $match);
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

  // Append the raw results to the big raw results file
  file_put_contents($appended_raw_results_file,
                    strtoupper($test_name)."\n\n".
                    file_get_contents($raw_results_file).
                    "\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n",
                    FILE_APPEND);

  // Make sure we are overwriting test results if they exit already,
  // as opposed to having duplicate test names
  $summary_line = "$test_name=$pct_str ($raw_results_file)".PHP_EOL;
  if (!(file_exists($summary_file))) {
    file_put_contents($summary_file, $summary_line);
  } else {
    $file_data = file($summary_file);
    $duplicate_match = preg_grep("/$test_name/", $file_data);
    if (!empty($duplicate_match)) {
      $file_data = preg_replace("/$test_name=.*".PHP_EOL."/",
                   $summary_line, $file_data);
    } else {
      $file_data[] = $summary_line;
    }
    file_put_contents($summary_file, $file_data);
  }

  return $pct_str;
}

function help(): void {
  $intro = <<<INTRO
  oss_framework_test_script

    This script runs one or more open source (oss) framework unit tests
    via PHPUnit. Run one or multiple tests by explicitly naming them at the
    command line, or specify --all to run all available tests.

INTRO;

  $examples = <<<EXAMPLES
  Usage:

    # Run all framework tests.
    % hhvm oss_framework_test_script --all

    # Run all framework tests using zend for alternative options for downloads.
    % hhvm oss_framework_test_script --all --zend ~/zend55/bin/php

    # Run all framework tests forcing the download of all the tests again.
    % hhvm oss_framework_test_script --all --redownload

    # Run all framework tests with a timeout per test (in secs).
    % hhvm oss_framework_test_script --all --timeout 600

    # Run one test.
    % hhvm oss_framework_test_script composer

    # Run multiple tests.
    % hhvm oss_framework_test_script composer assetic paris

    # Run multiple tests with timeout for each test (in seconds).
    # Tests must come after the -- options
    % hhvm oss_framework_test_script --timeout 600 composer assetic

    # Run multiple tests with timeout for each test (in seconds) and
    # with verbose messages. Tests must come after the -- options.
    % hhvm oss_framework_test_script --timeout 600 --verbose composer assetic

    # Display help.
    % hhvm oss_framework_test_script --help

EXAMPLES;

  display_help($intro, oss_test_option_map());
  echo $examples;

}

function oss_test_option_map(): OptionInfoMap {
  return Map {
    'help'                => Pair {'h', "Print help message"},
    'all'                 => Pair {'',  "Run all framework tests. The tests ".
                                        "to be run are hardcoded in a Map in ".
                                        "this code."},
    'timeout:'            => Pair {'t', "Optional - The maximum amount of ".
                                        "time, in secs, to allow a test to ".
                                        "run."},
    'verbose'             => Pair {'v', "Optional - For a lot of messages ".
                                        "about what is going on."},
    'zend:'               => Pair {'z', "Optional - Try to use zend if ".
                                        "retrieving dependencies with hhvm ".
                                        "fails. Currently, zend must be ".
                                        "installed and the path to the zend ".
                                        "binary specified."},
    'redownload'          => Pair {'r', "Forces a redownload of the framework ".
                                        "code and dependencies."}
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

function remove_dir_recursive(string $root_dir) {
  $files = new RecursiveIteratorIterator(
             new RecursiveDirectoryIterator(
               $root_dir,
               RecursiveDirectoryIterator::SKIP_DOTS),
             RecursiveIteratorIterator::CHILD_FIRST);

  foreach ($files as $fileinfo) {
    $todo = $fileinfo->isDir() ? fun('rmdir') : fun('unlink');
    $todo($fileinfo->getRealPath());
  }
  rmdir($root_dir);
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

// Start from self and work down tree. Find first occurrence closest to root
// Works on files or directories.
//
// Note: Wanted to use builtin SPL for this, but it seems like the order cannot
// be guaranteed with their iterators. So found and used a sorted iterator class
// and sorted by the full path including file name.
function find_any_file_recursive(Set $filenames, string $root_dir,
                                 bool $just_path_to_file): ?string {

  $dit = new RecursiveDirectoryIterator($root_dir,
                                        RecursiveDirectoryIterator::SKIP_DOTS);
  $rit = new RecursiveIteratorIterator($dit);
  $sit = new SortedIterator($rit);

  foreach ($sit as $fileinfo) {
    if ($filenames->contains($fileinfo->getFileName())) {
      return $just_path_to_file
             ? $fileinfo->getPath()
             : $fileinfo->getPathName();
    }
  }

  return null;
}

function get_hhvm_build(): string {
  $fbcode_root_dir = __DIR__.'/../../..';
  if (!(file_exists($fbcode_root_dir."/_bin"))) {
    error("HHVM build doesn't exist. Did you build yet?");
  }
  return $fbcode_root_dir."/_bin/hphp/hhvm/hhvm";
}

function verbose(string $msg, bool $verbose): void {
  if ($verbose) {
    echo $msg;
  }
}

function fun(string $s): string {
  return $s;
}

function main(array $argv): void {
  $options = parse_options(oss_test_option_map());
  if ($options->containsKey('help')) {
    return help();
  }
  echo "Script running....Be patient as some tests take a while with a debug ".
       "build of HHVM\n";
  Tests::init();
  if (ProxyInformation::is_proxy_required()) {
    echo "Looks like proxy may be required. Setting to default FB proxy ".
         "values. Please change Map in ProxyInformation to correct values, ".
         "if necessary.\n";
  }
  // Don't send $argv[0] which just contains the program to run
  // Parse other possible options out in run()
  run($options, Vector::fromArray(array_slice($argv, 1)));
  echo "Script finished!!!\n";
}

main($argv);
