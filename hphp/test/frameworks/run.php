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
 *   - Timeout option for running tests. Thre is a default of 30 minutes to
 *     run each test, but this can be shortened or lengthened as desired.
 *
 *   - Enhanced data output by the script to include "diff" type information
 *     about why a passing percentage is different from a previous run,
 *     particularly from a regression perspective. For example, what tests
 *     caused the regression.
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
include_once $FBCODE_ROOT.'/hphp/tools/command_line_lib.php';

type Result = int;
class TestResult {
  const Result FATAL = 10;
  const Result ERRORS = 11;
  const Result CLEAN = 12;
}

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

class PHPUnitPatterns {
  // Matches:
  //   Starting test 'PrettyExceptionsTest::testReturnsDiagnostics'.
  //   Starting test 'Assetic\Test\::testMethods with data set #1 ('getRoot')'.
  static string $test_name_pattern =
 "/Starting test '([_a-zA-Z0-9\\\\]*::[_a-zA-Z0-9]*( with data set #[0-9]+)?)/";

  // Matches:
  //    E
  //    .
  //    .  252 / 364 ( 69%)
  // Four \\\\ needed to match one \
  // stackoverflow.com/questions/4025482/cant-escape-the-backslash-with-regex
  static string $status_code_pattern =
                "/^[\.SFEI]$|^[\.SFEI][ \t]*[0-9]* \/ [0-9]* \([ 0-9]*%\)/";

  // Get rid of codes like ^[[31;31m that may get output to the results file.
  // 0x1B is the hex code for the escape sequence ^[
  static string $color_escape_code_pattern =
                "/\x1B\[([0-9]{1,2}(;[0-9]{1,2})?)?[m|K]/";

  // Don't want to parse any more test names after the Time line in the
  // results. Any test names after that line are probably detailed error
  // information.
  static string $stop_parsing_pattern =
                "/^Time: \d+.\d+ seconds, Memory: \d+.\d+/";

  static string $tests_ok_pattern = "/OK \(\d+ tests, \d+ assertions\)/";
  static string $tests_failure_pattern = "/Tests: \d+, Assertions: \d+.*[.]/";
}

class Frameworks {

  public static Map $framework_info = null;

  public static function init(): void {
    // HERE ARE THE CURRENTLY AVAILABLE TESTS.
    // Have a 'git_commit' field to ensure consistency across test runs as we
    // may have different download times for people, as well as redownloads.
    // The latest SHA at the time was used for the value.
    //
    // Some tests are commented out because they may need to be special cased as
    // they require special testing commands, mock databases, etc.
    //
    // IF WE HAVE A BLACKLIST FOR FLAKEY TESTS THAT PASS OR FAIL DEPENDING ON
    // THE POSITION OF THE MOON (OR AN UKNOWN BUG IN HHVM), THESE ARE THE
    // CANDIDATES:
    //
    // Joomla    - Joomla\Crypt\Tests\PasswordSimpleTest::testCreate
    //           - Joomla\Crypt\Tests\PasswordSimpleTest::testVerify
    // Slim      - SlimHttpUtilTest::
    //     testSetCookieHeaderWithNameAndValueAndDomainAndPathAndExpiresAsString
    // Symfony   - Symfony\Component\BrowserKit\Tests\ClientTest::testClick
    //           - Symfony\Component\BrowserKit\Tests\ClientTest::testClickForm
    //           - Symfony\Component\BrowserKit\Tests\ClientTest::testSubmit
    //           - Symfony\Component\BrowserKit\Tests\ClientTest::
    //                                                 testSubmitPreserveAuth
    //           - Symfony\Component\DomCrawler\Tests\CrawlerTest::*
    //
    self::$framework_info =
      Map {
        'assetic' =>
          Map {
            'install_root' => __DIR__."/frameworks/assetic",
            'git_path' => "git@github.com:kriswallsmith/assetic.git",
            'git_commit' => "e0646fa52937c4e5ce61ce089ada28c509b01b40",
          },
        'paris' =>
          Map {
            'install_root' => __DIR__."/frameworks/paris",
            'git_path' => "git@github.com:j4mie/paris.git",
            'git_commit' => "b60d0857d10dec757427b336c427c1f13b6a5e48",
          },
        'idiorm' =>
          Map {
            'install_root' => __DIR__."/frameworks/idiorm",
            'git_path' => "git@github.com:j4mie/idiorm.git",
            'git_commit' => "3be516b440734811b58bb9d0b458a4109b49af71",
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
            'git_commit' => "98c0d38a440e91adeb0ac12928174046596cd8e1",
          },
        'codeigniter' =>
          Map {
            'install_root' => __DIR__."/frameworks/CodeIgniter",
            'git_path' => "git@github.com:EllisLab/CodeIgniter.git",
            'git_commit' => "57ba100129c2807153d88dc4e1d423f6e6c8a9a6",
          },
        'laravel' =>
          Map {
            'install_root' => __DIR__."/frameworks/laravel",
            'git_path' => "git@github.com:laravel/framework.git",
            'git_commit' => "6ea8d8b5b3c921e9fe02bfafa44d2601d206ed6e",
          },
        'zf2' =>
          Map {
            'install_root' => __DIR__."/frameworks/zf2",
            'git_path' => "git@github.com:zendframework/zf2.git",
            'git_commit' => "3bd643acb98a5f6a9e5abd45785171f6685b4a3c",
          },
        'yii' =>
          Map {
            'install_root' => __DIR__."/frameworks/yii",
            'git_path' => "git@github.com:yiisoft/yii.git",
            'git_commit' => "d36b1f58ded2deacd4c5562c5205871db76bde5d",
            'test_path' => __DIR__."/frameworks/yii/tests",
            'test_run_command' => get_hhvm_build()." ".__DIR__.
                                  "/vendor/bin/phpunit --debug .",
          },
        'slim' =>
          Map {
            'install_root' => __DIR__."/frameworks/Slim",
            'git_path' => "git@github.com:codeguy/Slim.git",
            'git_commit' => "2e540cc392644b9b5cdcc57e5576db5a92ca2149",
          },
        /*'wordpress' =>
          Map {
            'install_root' => __DIR__."/frameworks/wordpress-unit-tests",
            'git_path' => "git@github.com:kurtpayne/wordpress-unit-tests.git",
            'git_commit' => "a2820a710a6605cca06ae5191ce888c51b22b0fe",
          },*/
        'composer' =>
          Map {
            'install_root' => __DIR__."/frameworks/composer",
            'git_path' => "git@github.com:composer/composer.git",
            'git_commit' => "7defc95e4b9eded1156386b269a9d7d28fa73710",
          },
        'doctrine2' =>
          Map {
            'install_root' => __DIR__."/frameworks/doctrine2",
            'git_path' => "git@github.com:doctrine/doctrine2.git",
            'git_commit' => "bd7c7ebaf353f038fae2f828802ecda823190759",
            'pull_requests' =>
              Vector {
                Map {
                  'pull_dir' => __DIR__."/frameworks/doctrine2".
                                 "/vendor/doctrine/dbal",
                  'pull_repository' => "https://github.com/javer/dbal",
                  'git_commit' => "hhvm-pdo-implement-interfaces",
                 },
              }
          },
        'twig' =>
          Map {
            'install_root' => __DIR__."/frameworks/Twig",
            'git_path' => "git@github.com:fabpot/Twig.git",
            'git_commit' => "d827c601e3afea6535fede5e39c9f91c12fc2e66",
          },
        'joomla' =>
          Map {
            'install_root' => __DIR__."/frameworks/joomla-framework",
            'git_path' => "git@github.com:joomla/joomla-framework.git",
            'git_commit' => "4669cd3b123e768f55545acb284bee666ce778c4",
          },
        /* Requires some sort of database setup.
        'magento2' =>
          Map {
            'install_root' => __DIR__."/frameworks/magento2",
            'git_path' => "git@github.com:magento/magento2.git",
            'git_commit' => "a15ecb31976feb4ecb62f85257ff6b606fbdbc00",
          },*/
        'phpmyadmin' =>
          Map {
            'install_root' => __DIR__."/frameworks/phpmyadmin",
            'git_path' => "git@github.com:phpmyadmin/phpmyadmin.git",
            'git_commit' => "6706fc1f9a7e8893cbe2672e9f26b30b3c49da52",
          },
        'phpbb3' =>
          Map {
            'install_root' => __DIR__."/frameworks/phpbb3",
            'git_path' => "git@github.com:phpbb/phpbb3.git",
            'git_commit' => "80b21e8049a138d07553288029abf66700da9a5c",
          },
        /*'pear' =>
          Map {
            'install_root' => __DIR__."/frameworks/pear-core",
            'git_path' => "git@github.com:pear/pear-core.git",
            'git_commit' => "9efe6005fd7a16c56773248d6878deec93481d39",
            'test_path' => __DIR__."/frameworks/pear-core",
            'test_run_command' => get_hhvm_build()." ".__DIR__.
                                  "/vendor/bin/phpunit --debug tests",
          },*/
        /* Requires a mediawiki install to run tests :(
        'mediawiki' =>
          Map {
            'install_root' => __DIR__."/frameworks/mediawiki-core",
            'git_path' => "git@github.com:wikimedia/mediawiki-core.git",
            'git_commit' => "7b9549088534ac636d131e054631adb16b39bd1e",
          },*/
        'typo3' =>
          Map {
            'install_root' => __DIR__."/frameworks/typo3",
            'git_path' => "git@github.com:TYPO3/TYPO3.CMS.git",
            'git_commit' => "085ca118bcb08213732c9e15462b6e2c073665e4",
            'test_path' => __DIR__."/frameworks/typo3/typo3",
            'test_run_command' => get_hhvm_build()." cli_dispatch.phpsh ".
                                  __DIR__."/vendor/bin/phpunit --debug",
          },
        'drupal' =>
          Map {
            'install_root' => __DIR__."/frameworks/drupal",
            'git_path' => "git@github.com:drupal/drupal.git",
            'git_commit' => "adaf8355074ba3e142f61e10f1790382db5defb9",
          },
        /*
        'twitteroauth' =>
          Map {
            'install_root' => __DIR__."/frameworks/twitteroauth",
            'git_path' => "git@github.com:abraham/twitteroauth.git",
            'git_commit' => "4b775766fe3526ebc67ee20c97ff29a3b47bc5d8",
          },*/
        /*'thinkup' =>
          Map {
            'install_root' => __DIR__."/frameworks/thinkup",
            'git_path' => "git@github.com:ginatrapani/ThinkUp.git",
            'git_commit' => "ae84fd6522ab0f15c36fd99e7bd55cef3e3ed90b",
            'test_path' => __DIR__."/frameworks/thinkup/tests",
            'test_run_command' => get_hhvm_build()." all_tests.php",
          },*/
        /*'cakephp' =>
          Map {
            'install_root' => __DIR__."/frameworks/cakephp",
            'git_path' => "git@github.com:cakephp/cakephp.git",
            'git_commit' => "bb4716a9ee628e15bf5854fa4e202e498591ec46",
            'test_path' => __DIR__."/frameworks/cakephp",
            // FIX: May have to update "cake" script to call "hhvm"
            'test_run_command' => "lib/Cake/Console/cake test core AllTests",
          },*/
        'facebook-php-sdk' =>
          Map {
            'install_root' => __DIR__."/frameworks/facebook-php-sdk",
            'git_path' => "git@github.com:facebook/facebook-php-sdk.git",
            'git_commit' => "16d696c138b82003177d0b4841a3e4652442e5b1",
            'test_path' => __DIR__."/frameworks/facebook-php-sdk",
            'test_run_command' => get_hhvm_build()." ".__DIR__.
                                  "/vendor/bin/phpunit --debug ".
                                  "--bootstrap tests/bootstrap.php ".
                                  "tests/tests.php",
          },
        'phpunit' =>
          Map {
            'install_root' => __DIR__."/frameworks/phpunit",
            'git_path' => "git@github.com:sebastianbergmann/phpunit.git",
            'git_commit' => "0ce13a8c9ff41d9c0d69ebd216bcc66b5f047246",
            'test_path' => __DIR__."/frameworks/phpunit",
            'test_run_command' => get_hhvm_build()." ".__DIR__.
                                  "/frameworks/phpunit/phpunit.php --debug",
          },
      };
  }
}

function prepare(OptionInfoMap $options, Vector $frameworks): void {
  Frameworks::init();

  // HACK: Yes, this next bit of "removeKey" code is hacky, maybe even clowny.
  // We can fix the command_line_lib.php to maybe make things a bit better.

  // It is possible that the $tests vector came in here with a combiniation
  // of command line options (e.g., verbose and timeout) that should be
  // removed before running the tests. Remeber all these option values are
  // already set in $options. They are just artificats of $argv right now.
  // Although, there is a failsafe when checking if the test exists that would
  // weed command line opts out too.
  $verbose = false;
  $csv_only = false;
  $csv_header = false;

  // Can't be both summary and verbose. Summary trumps.
  if ($options->containsKey('csv') && $options->containsKey('verbose')) {
    error("Cannot be --csv and --verbose together");
  }
  else if ($options->containsKey('csv')) {
    $csv_only = true;
    // $tests[0] may not even be "summary", but it doesn't matter, we are
    // just trying to make the count right for $tests
    $frameworks->removeKey(0);
  }
  else if ($options->containsKey('verbose')) {
    $verbose = true;
    $frameworks->removeKey(0);
  }

  if ($options->contains('csvheader')) {
    $csv_header = true;
    $frameworks->removeKey(0);
  }

  verbose("Script running....Be patient as some tests take a while with a ".
          "debug build of HHVM\n", $verbose);

  if (ProxyInformation::is_proxy_required()) {
    verbose("Looks like proxy may be required. Setting to default FB proxy ".
         "values. Please change Map in ProxyInformation to correct values, ".
         "if necessary.\n", $verbose);
  }

  $timeout = 1800; // 1800 seconds is the default run time before timeout
  if ($options->containsKey('timeout')) {
    $timeout = (int) $options['timeout'];
    // Remove timeout option and its value from the $tests vector
    $frameworks->removeKey(0);
    $frameworks->removeKey(0);
  }

  $zend_path = null;
  if ($options->containsKey('zend')) {
    verbose ("Will try Zend if necessary. If Zend doesn't work, the script ".
         "will still continue running; the particular framework on which Zend ".
         "was attempted may not be available though.\n", $verbose);
    $zend_path = $options['zend'];
    $frameworks->removeKey(0);
    $frameworks->removeKey(0);
  }

  $force_redownload = false;
  if ($options->containsKey('redownload')) {
    $force_redownload = true;
    $frameworks->removeKey(0);
  }

  $generate_new_expect_file = false;
  if ($options->containsKey('record')) {
    $generate_new_expect_file = true;
    $frameworks->removeKey(0);
  }

  get_unit_testing_infra_dependencies($csv_only);

  if ($options->containsKey('all')) {
    $frameworks->removeKey(0);
    // At this point, $tests should be empty if we are in --all mode.
    if (!($frameworks->isEmpty())) {
      error("Do not specify both --all and tests to run at the same time.\n");
    }
    // Running all tests
    run_test_suites(Frameworks::$framework_info->keys(), $timeout, $verbose,
              $csv_only, $csv_header, $force_redownload,
              $generate_new_expect_file, $zend_path);
  } else if (count($frameworks) > 0) {
    // Tests specified at the command line. At this point, $tests should only
    // have tests to be run
    run_test_suites($frameworks, $timeout, $verbose, $csv_only, $csv_header,
                    $force_redownload, $generate_new_expect_file, $zend_path);
  } else {
    error("Specify tests to run or use --all");
  }
}

function get_unit_testing_infra_dependencies(bool $csv_only): void {
  // Install composer.phar
  if (!(file_exists(__DIR__."/composer.phar"))) {
    verbose("Getting composer.phar....\n", !$csv_only);
    $comp_url = "http://getcomposer.org/composer.phar";
    $get_composer_command = "wget ".$comp_url." -P ".__DIR__." 2>&1";
    $ret = run_install($get_composer_command, __DIR__,
                       ProxyInformation::$proxies, $csv_only);
    if ($ret !== 0) {
      error("Could not download composer. Script stopping\n");
    }
  }

  // Install phpunit from composer.json located in __DIR__
  $phpunit_binary = __DIR__."/vendor/bin/phpunit";
  if (!(file_exists($phpunit_binary))) {
    verbose("\nDownloading PHPUnit in order to run tests. There may be an ".
            "output delay while the download begins.\n", !$csv_only);
    $phpunit_install_command = get_hhvm_build()." ".__DIR__.
                               "/composer.phar install --dev --verbose 2>&1";
    $ret = run_install($phpunit_install_command, __DIR__,
                       ProxyInformation::$proxies, $csv_only);
    if ($ret !== 0) {
      error("Could not install PHPUnit. Script stopping\n");
    }
  }

}

// This function should only be called once for a framework (assuming you don't
// delete the framework from your repo). The proxy could make things a bit
// adventurous, so we will see how this works out after some time to test it
// out
function install_framework(string $name, bool $verbose, bool $csv_only,
                           ?string $zend_path): void {
  verbose("Installing $name. You will see white dots during install.....\n",
          !$csv_only);
  $install_root = Frameworks::$framework_info[$name]['install_root'];

  /*******************************
   *       GIT CHECKOUT
   ******************************/

  // Get the source from GitHub
  verbose("Retrieving framework $name.....\n", $verbose);
  $git_command = "git clone";
  $git_command .= " ".Frameworks::$framework_info[$name]['git_path'];
  $git_command .= " ".$install_root;
  // "frameworks" directory will be created automatically on the first git clone
  // of a framework.
  $git_ret = run_install($git_command, __DIR__, ProxyInformation::$proxies,
                         $csv_only);
  if ($git_ret !== 0) {
    $csv_only ? error() : error("Could not download framework $name!\n");
  }
  // Checkout out our baseline test code via SHA
  $git_command = "git checkout";
  $git_command .= " ".Frameworks::$framework_info[$name]['git_commit'];
  $git_ret = run_install($git_command, $install_root,
                         ProxyInformation::$proxies, $csv_only);
  if ($git_ret !== 0) {
    $csv_only ? error()
              : error("Could not checkout baseline test code for $name!\n");
  }

  /*******************************
   *       FW DEPENDENCIES
   ******************************/

  $composer_json_path = find_any_file_recursive(Set {"composer.json"},
                                                $install_root, true);
  verbose("composer.json found in: $composer_json_path\n", $verbose);
  // Check to see if composer dependencies are necessary to run the test
  if ($composer_json_path !== null) {
    verbose("Retrieving dependencies for framework $name......\n", $verbose);
    $dependencies_install_cmd = get_hhvm_build()." ".__DIR__.
                                "/composer.phar install --dev";
    $install_ret = run_install($dependencies_install_cmd, $composer_json_path,
                               ProxyInformation::$proxies, $csv_only);
    // If provided the option at the command line, try Zend if we are not
    // successful with hhvm. For example, I know hhvm had trouble
    // downloading dependencies for Symfony, but Zend worked.
    //
    // FIX: Should we try to install a vanilla zend binary here instead of
    // relying on user to specify a path? Should we try to determine if zend
    // is already installed via a $PATH variable?
    if ($zend_path !== null && $install_ret !== 0) {
       verbose("HHVM didn't work for downloading dependencies for $name. ".
               "Trying Zend.\n", !$csv_only);
      $dependencies_install_cmd = $zend_path." ".__DIR__.
                                        "/composer.phar install --dev";
      $install_ret = run_install($dependencies_install_cmd, $composer_json_path,
                                 ProxyInformation::$proxies, $csv_only);
    }

    if ($install_ret !== 0) {
      // Let's just really make sure the dependencies didn't get installed
      // by checking the vendor directories to see if they are empty.
      $fw_vendor_dir = find_any_file_recursive(Set {"vendor"},
                                               $install_root,
                                               false);
      if ($fw_vendor_dir !== null) {
        // If there is no content in the directories under vendor, then we
        // did not get the dependencies.
        if (any_dir_empty_one_level($fw_vendor_dir)) {
          remove_dir_recursive($install_root);
          $csv_only ? error()
                    : error("Couldn't download dependencies for $name!".
                            " Removing framework. You can try the --zend".
                            " option.\n");
        }
      } else { // No vendor directory. Dependencies could not have been gotten.
        remove_dir_recursive($install_root);
        $csv_only ? error()
                  : error("Couldn't download dependencies for $name!".
                          " Removing framework. You can try the --zend".
                          " option.\n");
      }
    }
  }

  /*******************************
   *  OTHER PULL REQUESTS
   ******************************/
  if (Frameworks::$framework_info[$name]->containsKey("pull_requests")) {
    verbose("Merging some upstream pull requests for ".$name."\n", $verbose);
    $pull_requests = Frameworks::$framework_info[$name]["pull_requests"];
    foreach ($pull_requests as $pr) {
      $dir = $pr["pull_dir"];
      $rep = $pr["pull_repository"];
      $gc = $pr["git_commit"];
      chdir($dir);
      // Checkout out our baseline test code via SHA
      $git_command = "git pull --no-edit ".$rep." ".$gc;
      $git_ret = run_install($git_command, $dir,
                             ProxyInformation::$proxies, $csv_only);
      if ($git_ret !== 0) {
        $csv_only ? error()
                  : error("Could not checkout baseline test code for $name!\n");
      }
      chdir(__DIR__);
    }
  }
}

// This will run processes that will get the test infra dependencies
// (e.g. PHPUnit), frameworks and framework dependencies.
function run_install(string $proc, string $path, ?Map $env,
                     bool $csv_only): ?int
{
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
    while (fgets($pipes[1])) {
      if ((microtime(true) - $start_time) > 1) {
        verbose(".", !$csv_only);
        $start_time = microtime(true);
      }
    }
    fclose($pipes[1]);
    $ret = proc_close($process);
    return $ret;
  }
  return null;
}

function run_test_suites(Vector $frameworks, int $timeout, bool $verbose,
                         bool $csv_only, bool $csv_header,
                         bool $force_redownload, bool $generate_new_expect_file,
                         ?string $zend_path): void {
  $results_root = __DIR__."/results";
  $temp_summary_file = tempnam("/tmp", "oss-fw-test-summary");
  $frameworks_to_run = Vector {};
  foreach ($frameworks as $framework) {
    $name = trim(strtolower($framework));
    if (Frameworks::$framework_info->containsKey($name)) {
      $frameworks_to_run[] = $name;
    }
  }

  if (count($frameworks_to_run) > 0) {
    verbose("Beginning the unit tests.....\n", !$csv_only);
    $num_threads = min(count($frameworks_to_run), num_cpus() + 1);

    $children = Vector {};
    for ($i = 0; $i < $num_threads; $i++) {
      $pid = pcntl_fork();
      if ($pid === -1) {
        error('Issues creating threads for tests');
      } else if ($pid) {
        $children[] = $pid;
      } else {
        exit(run_single_test_suite($frameworks_to_run[$i], $temp_summary_file,
                                   $timeout, $verbose, $csv_only,
                                   $force_redownload, $generate_new_expect_file,
                                   $zend_path));
      }
    }

    $thread_ret_val = 0;
    $status = -1;
    foreach($children as $child) {
      pcntl_waitpid($child, $status);
      $thread_ret_val |= pcntl_wexitstatus($status);
    }

    if ($thread_ret_val === 0) {
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
     verbose($msg, !$csv_only);
    } else {
      $msg = "\nAll tests did not run as expected.\n\n".<<<THUMBSDOWN
        ______
       (( ____ \-
       (( _____
       ((_____
       ((____   ----
            /  /
           (_((
THUMBSDOWN
."\n";
      verbose($msg, !$csv_only);
      // Print the diffs
      if (!$csv_only) {
        foreach($frameworks as $fw_name) {
          $fn = $results_root."/".$fw_name.".diff";
          // The file may not exist or the file may not have anything in it
          // since there is no diff (i.e., all tests for that particular
          // framework ran as expected). Either way, don't print anything
          // out for those cases.
          if (file_exists($fn) && ($contents = file_get_contents($fn)) !== "") {
            print PHP_EOL."********* ".strtoupper($fw_name).
                  " **********".PHP_EOL;
            print $contents;
            print "To run tests for ".strtoupper($fw_name).
                  " use this command: \n";
            $dir = Frameworks::$framework_info[$fw_name]->
                   containsKey("test_path")
                   ? Frameworks::$framework_info[$fw_name]["test_path"]
                   : Frameworks::$framework_info[$fw_name]['install_root'];
            $command = "cd ".$dir." && ";
            $command .= prepare_framework_test_run_command($fw_name, $timeout);
            print $command." [TEST NAME]".PHP_EOL;
          }
        }
      }
    }

    if (file_exists($temp_summary_file)
        && ($contents = file_get_contents($temp_summary_file)) !== "") {
      $file_data = file_get_contents($temp_summary_file);
      $decoded_results = json_decode($file_data, true);
      ksort($decoded_results);

      if ($csv_only) {
        $not_first = false; // To avoid trailing comma
        if ($csv_header) {
          print "date,";
          foreach ($decoded_results as $key => $value) {
            if ($not_first) {
              print ",";
            }
            $not_first = true;
            print $key;
          }
          print PHP_EOL;
          $not_first = false;
        }
        print date("Y-m-d").",";
        foreach ($decoded_results as $key => $value) {
          if ($not_first) {
            print ",";
          }
          $not_first = true;
          print $value;
        }
      } else {
        print "\nALL TESTS COMPLETE!\n";
        print "SUMMARY:\n";
        foreach ($decoded_results as $key => $value) {
          print $key."=".$value.PHP_EOL;
        }
      }
    } else {
      print "\nNO SUMMARY INFO AVAILABLE!\n";
    }
  } else {
    error("No frameworks to tests!");
  }
}

function run_single_test_suite(string $fw_name, string $summary_file,
                               int $timeout, bool $verbose, bool $csv_only,
                               bool $force_redownload,
                               bool $generate_new_expect_file,
                               ?string $zend_path): ?int {

  /***********************************
   *  Make sure we have the framework
   *  installed.
   **********************************/
  $install_root = Frameworks::$framework_info[$fw_name]['install_root'];
  $git_head_file = Frameworks::$framework_info[$fw_name]['install_root'].
                   "/.git/HEAD";
  if (!(file_exists($install_root))) {
    install_framework($fw_name, $verbose, $csv_only, $zend_path);
  } else if ($force_redownload) {
    verbose("Forced redownloading of $fw_name...", !$csv_only);
    remove_dir_recursive($install_root);
    install_framework($fw_name, $verbose, $csv_only, $zend_path);
  // The commit hash has changed and we need to redownload
  } else if (trim(file_get_contents($git_head_file)) !==
             Frameworks::$framework_info[$fw_name]['git_commit']) {
    verbose("Redownloading $fw_name because git commit has changed...",
            !$csv_only);
    remove_dir_recursive($install_root);
    install_framework($fw_name, $verbose, $csv_only, $zend_path);
  }

  /***********************************
   *  Initial preparation
   **********************************/
  $run_command = prepare_framework_test_run_command($fw_name, $timeout);
  $test_path = get_test_path($fw_name, $install_root, $verbose);

  verbose("Running test for $fw_name\n", $verbose);
  verbose("Command: $run_command\n", $verbose);
  verbose("Test Path: $test_path\n", $verbose);

  $results_root = __DIR__."/results";
  if (!(file_exists($results_root))) {
    mkdir($results_root, 0755, true);
  }


  $results_file = $results_root."/".$fw_name.".out";
  $expect_file = $results_root."/".$fw_name.".expect";
  $diff_file = $results_root."/".$fw_name.".diff";
  $errors_file = $results_root."/".$fw_name.".errors";

  $pipes = null;
  $tn_matches = array();
  $match = null;
  $line = null;
  $status = null;
  $raw_results = "";
  $error_information = "";
  $diff_information = "";
  $ret_val = 0;
  $just_raw_results_and_errors = false;


  /***********************************
   * Have we run the tests before?
   * Are we generating a new expect
   * file?
   **********************************/
  $compare_tests = null;
  if (file_exists($expect_file)) {
    if ($generate_new_expect_file) {
      unlink($expect_file);
      verbose("Resetting the expect file. ".
              "Establishing new baseline with gray dots...\n", !$csv_only);
    } else {
      // Color codes would have already been stripped out. Don't need those.
      $compare_tests = get_fw_tests_to_run(
        $expect_file,
        PHPUnitPatterns::$test_name_pattern,
        PHPUnitPatterns::$status_code_pattern,
        PHPUnitPatterns::$stop_parsing_pattern
      );
      verbose(Colors::YELLOW . $fw_name . Colors::NONE . ": running. ".
              "Comparing against ".count($compare_tests)." tests\n",
              !$csv_only);
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
      verbose($run_msg, $verbose);
    }
  } else {
    verbose("First time running test suite for $fw_name. ".
            "Establishing baseline with gray dots...\n", !$csv_only);
  }

  /*************************************
   * Run the test suite
   ************************************/
  chdir($test_path);

  $descriptorspec = array(
    0 => array("pipe", "r"),
    1 => array("pipe", "w"),
    2 => array("pipe", "w"),
  );

  // $_ENV will passed in by default if the environment var is null
  $process = proc_open($run_command, $descriptorspec, $pipes, $test_path, null);
  if (is_resource($process)) {
    fclose($pipes[0]);
    while (!(feof($pipes[1]))) {
      $line = fgets($pipes[1]);
      $line = preg_replace(PHPUnitPatterns::$color_escape_code_pattern,
                           "", $line);
      $raw_results .= $line;
      if ($just_raw_results_and_errors) {
        // Don't output final test data in error file
        if (preg_match(PHPUnitPatterns::$tests_ok_pattern, $line) === 0 &&
            preg_match(PHPUnitPatterns::$tests_failure_pattern, $line) === 0) {
          $error_information .= $line;
        }
      }
      if (preg_match(PHPUnitPatterns::$stop_parsing_pattern, $line,
                     $sp_matches) === 1) {
        // Get rid of the next blank line after the stopping pattern
        fgets($pipes[1]);
        $just_raw_results_and_errors = true;
      }
      if (!$just_raw_results_and_errors &&
          preg_match(PHPUnitPatterns::$test_name_pattern, $line,
                     $tn_matches) === 1) {
        // The subpattern of the "Starting test" line is the actual test.
        $match = $tn_matches[1];
        do {
          $status = fgets($pipes[1]);
          $status = preg_replace(PHPUnitPatterns::$color_escape_code_pattern,
                                 "", $status);
          $raw_results .= $status;
          if (strpos($status, 'HipHop Fatal') !== false ||
              strpos($status, "hhvm:") !== false) {
            // We have hit a fatal or some nasty assert. Escape now and try to
            // get the results written.
            $error_information .= $status;
            break 2;
          }
        } while (!feof($pipes[1]) &&
                 preg_match(PHPUnitPatterns::$status_code_pattern,
                            $status) === 0);
        if ($compare_tests !== null && $compare_tests->containsKey($match)) {
          if (strlen($status) > 0) {
            $status = $status[0]; // In case we had "F 252 / 364 (69 %)"
          }
          if ($status === $compare_tests[$match]) {
            // FIX: posix_isatty(STDOUT) was always returning false, even though
            // can print in color. Check this out later.
            verbose(Colors::GREEN.".".Colors::NONE, !$csv_only);
          } else {
            $ret_val = -1;
            // We are different than we expected
            // Red if we go from pass to something else
            if ($compare_tests[$match] === '.') {
              verbose(Colors::RED."F".Colors::NONE, !$csv_only);
            // Green if we go from something else to pass
            } else if ($status === '.') {
              verbose(Colors::GREEN."F".Colors::NONE, !$csv_only);
            // Blue if we go from something "faily" to something "faily"
            // e.g., E to I or F
            } else {
              verbose(Colors::BLUE."F".Colors::NONE, !$csv_only);
            }
            verbose(PHP_EOL."Different status in ".$fw_name." for test ".
                    $match.PHP_EOL,!$csv_only);
            $diff_information .= "----------------------".PHP_EOL;
            $diff_information .= $match.PHP_EOL.PHP_EOL;
            $diff_information .= "EXPECTED: ".$compare_tests[$match].PHP_EOL;
            $diff_information .= ">>>>>>>".PHP_EOL;
            $diff_information .= "ACTUAL: ".$status.PHP_EOL.PHP_EOL;
          }
        } else {
          // This is either the first time we run the unit tests, and all pass
          // because we are establishing a baseline. OR we have run the tests
          // before, but we are having an issue getting to the actual tests
          // (e.g., yii is one test suite that has behaved this way).
          if ($compare_tests !== null) {
            $ret_val = -1;
            verbose(Colors::LIGHTBLUE."F".Colors::NONE, !$csv_only);
            verbose(PHP_EOL."Different status in ".$fw_name." for test ".
                    $match.PHP_EOL,!$csv_only);
            $diff_information .= "----------------------".PHP_EOL;
            $diff_information .= "Problem loading: ".$match.PHP_EOL.PHP_EOL;
          } else {
            verbose(Colors::GRAY.".".Colors::NONE, !$csv_only);
          }
        }
      }
      // Use this if we hit a fatal before we match any tests. Normally
      // $line is "" if we fatal before then.
      $prev_line = $line;
    }

    // If we have fataled before any tests were run or test statuses were found,
    // assume most of the $raw_results was error information.
    if ($status === null) {
      $error_information .= $raw_results;
    }

    fclose($pipes[1]);
    fclose($pipes[2]);
    if (proc_close($process) === -1) {
      $ret_val = -1;
    }

    file_put_contents($results_file, $raw_results);
    file_put_contents($errors_file, $error_information);
    file_put_contents($diff_file, $diff_information);
    // If the first baseline run, make both the same.
    if (!file_exists($expect_file)) {
      copy($results_file, $expect_file);
    }

   /****************************************
    * Test complete. Create results for
    * the framework that just ran.
    ****************************************/
    $pct = get_framework_pass_percentage($fw_name, $results_file, $verbose);
    $encoded_result = json_encode(array($fw_name => $pct));
    if (!(file_exists($summary_file))) {
      file_put_contents($summary_file, $encoded_result);
    } else {
      $file_data = file_get_contents($summary_file);
      $decoded_results = json_decode($file_data, true);
      $decoded_results[$fw_name] = $pct;
      file_put_contents($summary_file, json_encode($decoded_results));
    }
  } else {
    $csv_only ? error()
                  : error("Could not open process to run test for framework ".
                          $fw_name);
  }
  chdir(__DIR__);
  return $ret_val;
}

function prepare_framework_test_run_command(string $fw_name,
                                            int $timeout): string {
  // Remember, timeouts are in minutes.
  $run_command = __DIR__."/../../tools/timeout.sh -t $timeout";
  if (Frameworks::$framework_info[$fw_name]
      ->contains('test_run_command')) {
    $run_command .= " ".
                 Frameworks::$framework_info[$fw_name]['test_run_command'];
  } else {
    $run_command .= " ".get_hhvm_build()." ".__DIR__.
                    "/vendor/bin/phpunit --debug";
  }
  $run_command .= " 2>&1";
  return $run_command;
}

function get_test_path(string $fw_name, string $install_root, bool $verbose) {
  // 2 possibilities, phpunit.xml and phpunit.xml.dist for configuration
  $phpunit_config_files = Set {'phpunit.xml', 'phpunit.xml.dist'};

  $test_path = null;
  if (Frameworks::$framework_info[$fw_name]->contains('test_path')) {
    $test_path = Frameworks::$framework_info[$fw_name]['test_path'];
  } else {
    $phpunit_config_file_loc = null;
    $phpunit_config_file_loc = find_any_file_recursive($phpunit_config_files,
                                                       $install_root, true);
    // The test path will be where the phpunit config file is located since
    // that file contains the test directory name. If no config file, error.
    $test_path = $phpunit_config_file_loc !== null
               ? $phpunit_config_file_loc
               : error("No phpunit test directory found for $fw_name");
    verbose("Using phpunit xml file in: $phpunit_config_file_loc\n", $verbose);
  }
  return $test_path;
}

function get_fw_tests_to_run(string $expect_file, string $test_name_pattern,
                             string $status_code_pattern,
                             string $stop_parsing_pattern): ?Map {
  $file = fopen($expect_file, "r");

  $matches = array();
  $line = null;
  $tests = Map {};

  while (!feof($file)) {
    $line = fgets($file);
    if (preg_match($stop_parsing_pattern, $line, $matches) === 1) {
      break;
    }
    if (preg_match($test_name_pattern, $line, $matches) === 1) {
      do {
        // Get the next line for the expected status for that test
        $status = fgets($file);
      } while (!feof($file) &&
               preg_match($status_code_pattern, $status) === 0);
      // The second match will be the test name only from the "Starting test..."
      // match string
      $tests[$matches[1]] = $status[0];
    }
  }
  if ($tests->isEmpty()) {
    return null;
  }
  return $tests;
}

// We may have to special case frameworks that don't use
// phpunit for their testing (e.g. ThinkUp)
function get_framework_pass_percentage(string $name,
                                       string $results_file,
                                       bool $verbose): mixed {
  // Get the last 4 lines of the results file which will give us the surface
  // area necessary to get the final stats.
  $file = escapeshellarg($results_file);
  if (($lines = `tail -n 4 $file`) === "") {
    error("Error occured. Probably could not open final ".
          "pass/fail file: $results_file");
  }

  // clean pattern represents: OK (364 tests, 590 assertions)
  // error pattern represents: Tests: 364, Assertions: 585, Errors: 5.
  $possible_patterns =
      Map {TestResult::CLEAN => PHPUnitPatterns::$tests_ok_pattern,
           TestResult::ERRORS => PHPUnitPatterns::$tests_failure_pattern};

  $test_result = TestResult::FATAL;
  $match = array();
  foreach ($possible_patterns as $result => $pattern) {
    if (preg_match($pattern, $lines, $match) === 1) {
      $test_result = $result;
      break;
    }
  }

  $pct = "";
  switch ($test_result) {
    case TestResult::CLEAN:
      $pct = "100";
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
      break;
    case TestResult::FATAL:
      $pct = "Fatal";
      break;
    default:
      $pct = "Unknown";
      break;
  }

  verbose(strtoupper($name).
          " TEST COMPLETE with pass percentage of: $pct\n\n", $verbose);
  verbose("Results File: $results_file\n", $verbose);
  verbose("..........\n\n", $verbose);

  return $pct;
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

    # Run all framework tests with a timeout per test (in secs).
    % hhvm run.php --all --timeout 600

    # Run one test.
    % hhvm run.php composer

    # Run multiple tests.
    % hhvm run.php composer assetic paris

    # Run multiple tests with timeout for each test (in seconds).
    # Tests must come after the -- options
    % hhvm run.php --timeout 600 composer assetic

    # Run multiple tests with timeout for each test (in seconds) and
    # with verbose messages. Tests must come after the -- options.
    % hhvm run.php --timeout 600 --verbose composer assetic

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
                                        "code and dependencies."},
    'record'              => Pair {'e', "Forces a new expect file for the ".
                                        "framework test suite"},
    'csv'                 => Pair {'c', "Just create the machine readable ".
                                        "summary CSV for parsing and chart ".
                                        "display."},
    'csvheader'           => Pair {'',  "Add a header line for the summary ".
                                        "CSV which includes the framework ".
                                        "names."}
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

  // This can be better, but good enough for now.
  // Maybe just use rm -rf, but that always seems
  // a bit dangerous. The below is probably only
  // O(2n) or so. No order depth order guaranteed
  // with the iterator, so actual files can be
  // deleted before symlinks

  // Get rid of the symlinks first to avoid orphan
  // symlinks that cannot be deleted.
  foreach ($files as $fileinfo) {
    if (is_link($fileinfo)) {
      $target = readlink($fileinfo);
      unlink($fileinfo);
      unlink($target);
    }
  }

  // Get rid of the rest
  foreach ($files as $fileinfo) {
    if ($fileinfo->isDir()) {
      rmdir($fileinfo->getRealPath());
    } else {
      unlink($fileinfo->getRealPath());
    }
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

function get_hhvm_build(bool $with_jit = true): string {
  $fbcode_root_dir = __DIR__.'/../../..';
  $build = "";
  // See if we are using an internal development build
  if ((file_exists($fbcode_root_dir."/_bin"))) {
    $build .= $fbcode_root_dir."/_bin/hphp/hhvm/hhvm";
  } else if (command_exists("hhvm")) {
  // Maybe we are in OSS land trying this script
    $build .= "hhvm";
  } else {
    error("HHVM build doesn't exist. Did you build yet?");
  }
  if ($with_jit) {
    $build .= " -v Eval.Jit=true";
  }
  return $build;
}

function command_exists(string $cmd): bool {
    $ret = shell_exec("which $cmd");
    return (empty($ret) ? false : true);
}

function verbose(string $msg, bool $verbose): void {
  if ($verbose) {
    print $msg;
  }
}

function main(array $argv): void {
  $options = parse_options(oss_test_option_map());
  if ($options->containsKey('help')) {
    return help();
  }
  // Don't send $argv[0] which just contains the program to run
  // Parse other possible options out in run()
  prepare($options, Vector::fromArray(array_slice($argv, 1)));
}

main($argv);
