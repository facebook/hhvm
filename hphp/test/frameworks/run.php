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
  //   PrettyExceptionsTest::testReturnsDiagnostics
  //   Assetic\Test\::testMethods with data set #1 ('getRoot')'
  //   Composer\Test\::testMe with data set "parses dates w/ -"
  // The "with data set" can either have a # or " after it and then any char
  // before a resulting " or (
  // Four \\\\ needed to match one \
  // stackoverflow.com/questions/4025482/cant-escape-the-backslash-with-regex
  static string $test_name_pattern =
  "/[_a-zA-Z0-9\\\\]*::[_a-zA-Z0-9]*( with data set (\"|#)[^\"|^\(|^\n]+(\")?)?/";

  static string $pear_test_name_pattern =
 "/[\-_a-zA-Z0-9\/]*\.phpt/";

  // Matches:
  //    E
  //    .
  //    .  252 / 364 ( 69%)
  //    .HipHop Warning
  // That last example happened in Magento
  static string $status_code_pattern =
  "/^[\.SFEI]$|^[\.SFEI](HipHop)|^[\.SFEI][ \t]*[0-9]* \/ [0-9]* \([ 0-9]*%\)/";

  // Get rid of codes like ^[[31;31m that may get output to the results file.
  // 0x1B is the hex code for the escape sequence ^[
  static string $color_escape_code_pattern =
                "/\x1B\[([0-9]{1,2}(;[0-9]{1,2})?)?[m|K]/";

  // Don't want to parse any more test names after the Time line in the
  // results. Any test names after that line are probably detailed error
  // information.
  static string $stop_parsing_pattern =
          "/^Time: \d+(\.\d+)? (seconds|ms|minutes|hours), Memory: \d+(\.\d+)/";

  static string $tests_ok_pattern = "/OK \(\d+ test[s]?, \d+ assertion[s]?\)/";
  static string $tests_failure_pattern = "/Tests: \d+, Assertions: \d+.*[.]/";

  static string $header_pattern =
                "/PHPUnit \d+.[0-9a-zA-Z\-\.]*( by Sebastian Bergmann.)?/";

  static string $config_file_pattern = "/Configuration read from/";

  static string $xdebug_pattern = "/The Xdebug extension is not loaded./";

  static string $test_file_pattern = "/.*\.phpt|.*Test\.php|.*test\.php/";

  static string $tests_ok_skipped_inc_pattern =
               "/OK, but incomplete or skipped tests!/";
  static string $num_errors_failures_pattern =
               "/There (was|were) \d+ (failure|error)[s]?\:/";
  static string $num_skips_inc_pattern =
               "/There (was|were) \d+ (skipped|incomplete) test[s]?\:/";
  static string $failures_header_pattern = "/FAILURES!/";
  static string $no_tests_executed_pattern = "/No tests executed!/";

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
    // In order to get frameworks to work correctly, may need to grab more code
    // via some sort of pull request:
    //
    //  - pull:      The code we need is in a that doesn't affect the primary
    //               branch or SHA (e.g., 'vendor') and we can just do
    //               a 'git pull' since any branch or HEAD change doesn't matter
    //  - submodule: The code we are adding may be in the root framework dir
    //               so that can affect the framework branch or SHA. If we
    //               pull/merge, the HEAD SHA changes. (FIX IF THIS DOESN'T
    //               HAVE TO BE THE CASE). And, if that happens, we will always
    //               be redownloading the framework since the SHA is different
    //               than what we expect. Use a submodule/move technique.
    // IF WE HAVE A BLACKLIST FOR FLAKEY TESTS THAT PASS OR FAIL DEPENDING ON
    // THE POSITION OF THE MOON (OR AN UNKNOWN BUG IN HHVM), THESE ARE THE
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
            'git_path' => "https://github.com/kriswallsmith/assetic.git",
            'git_commit' => "e0646fa52937c4e5ce61ce089ada28c509b01b40",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/assetic/tests/Assetic/Test",
              },
          },
        'paris' =>
          Map {
            'install_root' => __DIR__."/frameworks/paris",
            'git_path' => "https://github.com/j4mie/paris.git",
            'git_commit' => "b60d0857d10dec757427b336c427c1f13b6a5e48",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/paris/test",
              },
          },
        'idiorm' =>
          Map {
            'install_root' => __DIR__."/frameworks/idiorm",
            'git_path' => "https://github.com/j4mie/idiorm.git",
            'git_commit' => "3be516b440734811b58bb9d0b458a4109b49af71",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/idiorm/test",
              },
          },
        'symfony' =>
          Map {
            'install_root' => __DIR__."/frameworks/symfony",
            'git_path' => "https://github.com/symfony/symfony.git",
            'git_commit' => "98c0d38a440e91adeb0ac12928174046596cd8e1",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/symfony/src/Symfony/Bridge/*/Tests",
                __DIR__."/frameworks/symfony/src/Symfony/Component/*/Tests/",
                __DIR__."/frameworks/symfony/src/Symfony/Component/*/*/Tests/",
              },
          },
        'codeigniter' =>
          Map {
            'install_root' => __DIR__."/frameworks/CodeIgniter",
            'git_path' => "https://github.com/EllisLab/CodeIgniter.git",
            'git_commit' => "57ba100129c2807153d88dc4e1d423f6e6c8a9a6",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/CodeIgniter/tests/codeigniter/core",
                __DIR__."/frameworks/CodeIgniter/tests/codeigniter/helpers",
                __DIR__."/frameworks/CodeIgniter/tests/codeigniter/libraries",
              },
          },
        'laravel' =>
          Map {
            'install_root' => __DIR__."/frameworks/laravel",
            'git_path' => "https://github.com/laravel/framework.git",
            'git_commit' => "6ea8d8b5b3c921e9fe02bfafa44d2601d206ed6e",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/laravel/tests",
              },
          },
        'zf2' =>
          Map {
            'install_root' => __DIR__."/frameworks/zf2",
            'git_path' => "https://github.com/zendframework/zf2.git",
            'git_commit' => "3bd643acb98a5f6a9e5abd45785171f6685b4a3c",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/zf2/tests/ZendTest",
              },
          },
        'yii' =>
          Map {
            'install_root' => __DIR__."/frameworks/yii",
            'git_path' => "https://github.com/yiisoft/yii.git",
            'git_commit' => "d36b1f58ded2deacd4c5562c5205871db76bde5d",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/yii/tests",
              },
          },
        // Using a branch from https://github.com/codeguy/Slim to access an
        // upstream hash_hmac fix
        'slim' =>
          Map {
            'install_root' => __DIR__."/frameworks/Slim",
            'git_path' => "https://github.com/elgenie/Slim.git",
            'git_commit' => "1beca31c1f0b0a7bb7747d9367fb07c07e190a8d",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/Slim/tests",
              },
          },
        /*'wordpress' =>
          Map {
            'install_root' => __DIR__."/frameworks/wordpress-unit-tests",
            'git_path' => "https://github.com/kurtpayne/".
                          "wordpress-unit-tests.git",
            'git_commit' => "a2820a710a6605cca06ae5191ce888c51b22b0fe",
          },*/
        'composer' =>
          Map {
            'install_root' => __DIR__."/frameworks/composer",
            'git_path' => "https://github.com/composer/composer.git",
            'git_commit' => "7defc95e4b9eded1156386b269a9d7d28fa73710",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/composer/tests/Composer",
              },
          },
        'doctrine2' =>
          Map {
            'install_root' => __DIR__."/frameworks/doctrine2",
            'git_path' => "https://github.com/doctrine/doctrine2.git",
            'git_commit' => "bd7c7ebaf353f038fae2f828802ecda823190759",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/doctrine2/tests/Doctrine/Tests/ORM",
              },
            // This pull request is a vendor pull that does not affect the
            // primary git sha above
            'pull_requests' =>
              Vector {
                Map {
                  'pull_dir' => __DIR__."/frameworks/doctrine2".
                                 "/vendor/doctrine/dbal",
                  'pull_repository' => "https://github.com/javer/dbal",
                  'git_commit' => "hhvm-pdo-implement-interfaces",
                  'type' => 'pull',
                 },
              },
          },
        'twig' =>
          Map {
            'install_root' => __DIR__."/frameworks/Twig",
            'git_path' => "https://github.com/fabpot/Twig.git",
            'git_commit' => "d827c601e3afea6535fede5e39c9f91c12fc2e66",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/Twig/test/Twig",
              },
          },
        'joomla' =>
          Map {
            'install_root' => __DIR__."/frameworks/joomla-framework",
            'git_path' => "https://github.com/joomla/joomla-framework.git",
            'git_commit' => "4669cd3b123e768f55545acb284bee666ce778c4",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/joomla-framework/src/Joomla/*/Tests",
              },
          },
        'magento2' =>
          Map {
            'install_root' => __DIR__."/frameworks/magento2",
            'git_path' => "https://github.com/magento/magento2.git",
            'git_commit' => "a15ecb31976feb4ecb62f85257ff6b606fbdbc00",
            'test_path' => __DIR__."/frameworks/magento2/dev/tests/unit",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/magento2/dev/tests/unit/testsuite",
              },
          },
        'phpmyadmin' =>
          Map {
            'install_root' => __DIR__."/frameworks/phpmyadmin",
            'git_path' => "https://github.com/phpmyadmin/phpmyadmin.git",
            'git_commit' => "6706fc1f9a7e8893cbe2672e9f26b30b3c49da52",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/phpmyadmin/test",
              },
          },
        'phpbb3' =>
          Map {
            'install_root' => __DIR__."/frameworks/phpbb3",
            'git_path' => "https://github.com/phpbb/phpbb3.git",
            'git_commit' => "80b21e8049a138d07553288029abf66700da9a5c",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/phpbb3/tests",
              },
          },
        'pear' =>
          Map {
            'install_root' => __DIR__."/frameworks/pear-core",
            'git_path' => "https://github.com/pear/pear-core.git",
            'git_commit' => "9efe6005fd7a16c56773248d6878deec93481d39",
            'test_path' => __DIR__."/frameworks/pear-core",
            'test_run_command' => get_hhvm_build()." ".__DIR__.
                                  "/vendor/bin/phpunit --debug",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/pear-core/tests",
              },
            'pull_requests' =>
              Vector {
                Map {
                  'pull_dir' => __DIR__."/frameworks/pear-core",
                  'pull_repository' => "https://github.com/pear/Console_Getopt",
                  'git_commit' => "trunk",
                  'type' => 'submodulemove',
                  'move_from_dir' => __DIR__.
                                     "/frameworks/pear-core/Console_Getopt",
                 },
              },
          },
        'mediawiki' =>
          Map {
            'install_root' => __DIR__."/frameworks/mediawiki-core",
            'git_path' => "https://github.com/wikimedia/mediawiki-core.git",
            'git_commit' => "8c5733c44977232ca42454ae7f1ae0fd01770b37",
            'test_path' => __DIR__."/frameworks/mediawiki-core/tests/phpunit",
            'test_run_command' => get_hhvm_build()." phpunit.php --debug ".
                                  "--exclude-group=Database",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/mediawiki-core/tests/phpunit",
              },
          },
        /*'typo3' =>
          Map {
            'install_root' => __DIR__."/frameworks/typo3",
            'git_path' => "https://github.com/TYPO3/TYPO3.CMS.git",
            'git_commit' => "085ca118bcb08213732c9e15462b6e2c073665e4",
            'test_path' => __DIR__."/frameworks/typo3",
            'test_run_command' => get_hhvm_build().
                                  " ./typo3/cli_dispatch.phpsh ".
                                  __DIR__."/vendor/bin/phpunit --debug",
          },*/
        'drupal' =>
          Map {
            'install_root' => __DIR__."/frameworks/drupal",
            'git_path' => "https://github.com/drupal/drupal.git",
            'git_commit' => "adaf8355074ba3e142f61e10f1790382db5defb9",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/drupal/core/tests",
                __DIR__."/frameworks/drupal/core/modules/*/tests/*",
                __DIR__."/frameworks/drupal/core/../modules/*/tests/*",
                __DIR__."/frameworks/drupal/core/../sites/*/modules/*/tests/*",
              },
          },
        /*
        'twitteroauth' =>
          Map {
            'install_root' => __DIR__."/frameworks/twitteroauth",
            'git_path' => "https://github.com/abraham/twitteroauth.git",
            'git_commit' => "4b775766fe3526ebc67ee20c97ff29a3b47bc5d8",
          },*/
        /*'thinkup' =>
          Map {
            'install_root' => __DIR__."/frameworks/thinkup",
            'git_path' => "https://github.com/ginatrapani/ThinkUp.git",
            'git_commit' => "ae84fd6522ab0f15c36fd99e7bd55cef3e3ed90b",
            'test_path' => __DIR__."/frameworks/thinkup/tests",
            'test_run_command' => get_hhvm_build()." all_tests.php",
          },*/
        /*'cakephp' =>
          Map {
            'install_root' => __DIR__."/frameworks/cakephp",
            'git_path' => "https://github.com/cakephp/cakephp.git",
            'git_commit' => "bb4716a9ee628e15bf5854fa4e202e498591ec46",
            'test_path' => __DIR__."/frameworks/cakephp",
            // FIX: May have to update "cake" script to call "hhvm"
            'test_run_command' => "lib/Cake/Console/cake test core AllTests",
          },*/
        'facebook-php-sdk' =>
          Map {
            'install_root' => __DIR__."/frameworks/facebook-php-sdk",
            'git_path' => "https://github.com/facebook/facebook-php-sdk.git",
            'git_commit' => "16d696c138b82003177d0b4841a3e4652442e5b1",
            'test_path' => __DIR__."/frameworks/facebook-php-sdk",
            'test_run_command' => get_hhvm_build()." ".__DIR__.
                                  "/vendor/bin/phpunit --debug ".
                                  "--bootstrap tests/bootstrap.php",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/facebook-php-sdk/tests",
              },
          },
        'phpunit' =>
          Map {
            'install_root' => __DIR__."/frameworks/phpunit",
            'git_path' => "https://github.com/sebastianbergmann/phpunit.git",
            'git_commit' => "236f65cc97d6beaa8fcb8a27b19bd278f3912677",
            'test_path' => __DIR__."/frameworks/phpunit",
            'test_run_command' => get_hhvm_build()." ".__DIR__.
                                  "/frameworks/phpunit/phpunit.php --debug",
            'test_search_roots' =>
              Vector {
                __DIR__."/frameworks/phpunit/Tests/Framework",
                __DIR__."/frameworks/phpunit/Tests/Extensions",
                __DIR__."/frameworks/phpunit/Tests/Regression",
                __DIR__."/frameworks/phpunit/Tests/Runner",
                __DIR__."/frameworks/phpunit/Tests/TextUI",
                __DIR__."/frameworks/phpunit/Tests/Util",
              },
            'env_vars' =>
              Map {
                'PHP_BINARY' => get_hhvm_build(false, true)
              },
          },
      };
  }
}

class Options {
  public static int $timeout;
  public static bool $verbose;
  public static bool $csv_only;
  public static bool $csv_header;
  public static bool $force_redownload;
  public static bool $generate_new_expect_file;
  public static string $zend_path;
  public static bool $all;
  public static bool $allexcept;

  public static function parse(OptionInfoMap $options, array $argv): Vector {
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
    $verbose = false;
    $csv_only = false;
    $csv_header = false;
    $all = false;
    $allexcept = false;

    // Can't run all the framework tests and "all but" at the same time
    if ($options->containsKey('all') && $options->containsKey('allexcept')) {
      error("Cannot use --all and --allexcept together");
    } else if ($options->containsKey('all')) {
      $all = true;
      $framework_names->removeKey(0);
    } else if ($options->containsKey('allexcept')) {
      $allexcept = true;
      $framework_names->removeKey(0);
    }

    // Can't be both summary and verbose. Summary trumps.
    if ($options->containsKey('csv') && $options->containsKey('verbose')) {
      error("Cannot be --csv and --verbose together");
    }
    else if ($options->containsKey('csv')) {
      $csv_only = true;
      // $tests[0] may not even be "summary", but it doesn't matter, we are
      // just trying to make the count right for $frameworks
      $framework_names->removeKey(0);
    }
    else if ($options->containsKey('verbose')) {
      $verbose = true;
      $framework_names->removeKey(0);
    }

    if ($options->contains('csvheader')) {
      $csv_header = true;
      $framework_names->removeKey(0);
    }

    verbose("Script running...Be patient as some frameworks take a while with ".
            "a debug build of HHVM\n", $verbose);

    if (ProxyInformation::is_proxy_required()) {
      verbose("Looks like proxy may be required. Setting to default FB proxy ".
           "values. Please change Map in ProxyInformation to correct values, ".
           "if necessary.\n", $verbose);
    }

    $timeout = 60; // seconds to run any individual test for any framework
    if ($options->containsKey('timeout')) {
      $timeout = (int) $options['timeout'];
      // Remove timeout option and its value from the $framework_names vector
      $framework_names->removeKey(0);
      $framework_names->removeKey(0);
    }

    $zend_path = null;
    if ($options->containsKey('zend')) {
      verbose ("Will try Zend if necessary. If Zend doesn't work, the script ".
           "will still continue; the particular framework on which Zend ".
           "was attempted may not be available though.\n", $verbose);
      $zend_path = $options['zend'];
      $framework_names->removeKey(0);
      $framework_names->removeKey(0);
    }

    $force_redownload = false;
    if ($options->containsKey('redownload')) {
      $force_redownload = true;
      $framework_names->removeKey(0);
    }

    $generate_new_expect_file = false;
    if ($options->containsKey('record')) {
      $generate_new_expect_file = true;
      $framework_names->removeKey(0);
    }

    self::$timeout = $timeout;
    self::$verbose = $verbose;
    self::$csv_only = $csv_only;
    self::$csv_header = $csv_header;
    self::$force_redownload = $force_redownload;
    self::$generate_new_expect_file = $generate_new_expect_file;
    self::$zend_path = $zend_path;
    self::$all = $all;
    self::$allexcept = $allexcept;

    // This will return just the name of the frameworks passed in, if any left
    // (e.g. --all may have been passed, in which case the Vector will be
    // empty)
    return $framework_names;
  }
}

class Framework {
  public string $name;

  public string $out_file;
  public string $expect_file;
  public string $diff_file;
  public string $errors_file;
  public string $fatals_file;
  public string $stats_file;

  public string $test_path;
  public string $install_root;
  public string $test_command;
  public Map $env_vars;

  public string $test_name_pattern;
  public Vector $test_search_roots;

  public ?Map $current_test_statuses = null;
  public Set $tests = null;

  public bool $success;

  public function __construct(string $name) {
    $this->name = $name;
    $this->setInstallRoot();
  }

  public function prepareOutputFiles(string $path): void {
    if (!(file_exists($path))) {
      mkdir($path, 0755, true);
    }
    $this->out_file = $path."/".$this->name.".out";
    $this->expect_file = $path."/".$this->name.".expect";
    $this->diff_file = $path."/".$this->name.".diff";
    $this->errors_file = $path."/".$this->name.".errors";
    $this->fatals_file = $path."/".$this->name.".fatals";
    $this->stats_file = $path."/".$this->name.".stats";
  }

  // We may have to special case frameworks that don't use
  // phpunit for their testing (e.g. ThinkUp)
  public function getPassPercentage(): mixed {
    if (filesize($this->stats_file) === 0) {
      verbose("Stats File: ".$this->stats_file." has no content. Returning ".
              "fatal\n", Options::$verbose);
      return "Fatal";
    }

    $num_tests = 0;
    $num_errors_failures = 0;

    // clean pattern represents: OK (364 tests, 590 assertions)
    // error pattern represents: Tests: 364, Assertions: 585, Errors: 5.
    $match = array();
    $handle = fopen($this->stats_file, "r");
    if ($handle) {
      while (($line = fgets($handle)) !== false) {
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
          // Removed skipped tests
          $num_tests +=
            (float)($parsed_results["Tests"] - $parsed_results["Skipped"]);
          $num_errors_failures +=
            (float)($parsed_results["Errors"] + $parsed_results["Failures"] +
            $parsed_results["Incomplete"]);
        } else if ($line === "FATAL") {
          // If we fatal on a test or test file, just assume 1 test that has
          // failed.
          $num_tests += 1;
          $num_errors_failures += 1;
        }
      }
    } else {
      // If we cannot open the stats file, return Fatal
      $pct = "Fatal";
    }

    if ($num_tests > 0) {
      $pct = round(($num_tests - $num_errors_failures) / $num_tests, 4) * 100;
    } else {
      $pct = "Fatal";
    }

    verbose(strtoupper($this->name).
            " TEST COMPLETE with pass percentage of: ".$pct."\n",
            Options::$verbose);
    verbose("Stats File: ".$this->stats_file."\n", Options::$verbose);

    return $pct;
  }

  // This function should only be called once for framework (assuming you don't
  // delete the framework from your repo). The proxy could make things a bit
  // adventurous, so we will see how this works out after some time to test it
  // out
  public function install(): void {
    if ($this->isInstalled()) {
      verbose($this->name." already installed.\n", Options::$verbose);
      return;
    }
    verbose("Installing ".$this->name.
            ". You will see white dots during install.....\n",
            !Options::$csv_only);

    /*******************************
     *       GIT CHECKOUT
     ******************************/

    // Get the source from GitHub
    verbose("Retrieving framework ".$this->name."....\n", Options::$verbose);
    $git_command = "git clone";
    $git_command .= " ".Frameworks::$framework_info[$this->name]['git_path'];
    $git_command .= " ".$this->install_root;
    // "frameworks" directory will be created automatically on first git clone
    // of a framework.
    $git_ret = run_install($git_command, __DIR__, ProxyInformation::$proxies);
    if ($git_ret !== 0) {
      Options::$csv_only ? error()
                         : error("Could not download framework ".
                                 $this->name."!\n");
    }
    // Checkout out our baseline test code via SHA
    $git_command = "git checkout";
    $git_command .= " ".
                    Frameworks::$framework_info[$this->name]['git_commit'];
    $git_ret = run_install($git_command, $this->install_root,
                           ProxyInformation::$proxies);
    if ($git_ret !== 0) {
      remove_dir_recursive($this->install_root);
      Options::$csv_only ? error()
                         : error("Could not checkout baseline code for ".
                                 $this->name."! Removing framework!\n");
    }

    /******************************
     *       MEDIAWIKI SPECIFIC
     ******************************/
    // Need to have an empty LocalSettings.php file for testing to work.
    if ($this->name === "mediawiki") {
      verbose("Adding LocalSettings.php file to Mediawiki test dir.\n",
              Options::$verbose);
      $touch_command = "touch ".$this->install_root."/LocalSettings.php";
      exec($touch_command);
    }

    /*******************************
     *       FW DEPENDENCIES
     ******************************/

    $composer_json_path = find_any_file_recursive(Set {"composer.json"},
                                                  $this->install_root, true);
    verbose("composer.json found in: $composer_json_path\n", Options::$verbose);
    // Check to see if composer dependencies are necessary to run the test
    if ($composer_json_path !== null) {
      verbose("Retrieving dependencies for framework ".$this->name.".....\n",
              Options::$verbose);
      // Use the timeout to avoid curl SlowTimer timeouts and problems
      $dependencies_install_cmd = get_hhvm_build()." ".
                                  "-v ResourceLimit.SocketDefaultTimeout=30 ".
                                  __DIR__.
                                  "/composer.phar install --dev";
      $install_ret = run_install($dependencies_install_cmd, $composer_json_path,
                                 ProxyInformation::$proxies);
      // If provided the option at the command line, try Zend if we are not
      // successful with hhvm. For example, I know hhvm had trouble
      // downloading dependencies for Symfony, but Zend worked.
      //
      // FIX: Should we try to install a vanilla zend binary here instead of
      // relying on user to specify a path? Should we try to determine if zend
      // is already installed via a $PATH variable?
      if (Options::$zend_path !== null && $install_ret !== 0) {
         verbose("HHVM didn't work for downloading dependencies for ".
                 $this->name." Trying Zend.\n", !Options::$csv_only);
        $dependencies_install_cmd = Options::$zend_path." ".__DIR__.
                                          "/composer.phar install --dev";
        $install_ret = run_install($dependencies_install_cmd,
                                   $composer_json_path,
                                   ProxyInformation::$proxies);
      }

      if ($install_ret !== 0) {
        // Let's just really make sure the dependencies didn't get installed
        // by checking the vendor directories to see if they are empty.
        $fw_vendor_dir = find_any_file_recursive(Set {"vendor"},
                                                 $this->install_root,
                                                 false);
        if ($fw_vendor_dir !== null) {
          // If there is no content in the directories under vendor, then we
          // did not get the dependencies.
          if (any_dir_empty_one_level($fw_vendor_dir)) {
            remove_dir_recursive($this->install_root);
            Options::$csv_only ? error()
                               : error("Couldn't download dependencies for ".
                                       $this->name." Removing framework. ".
                                       "You can try the --zend option.\n");
          }
        } else { // No vendor directory. Dependencies could not have been gotten
          remove_dir_recursive($this->install_root);
          Options::$csv_only ? error()
                             : error("Couldn't download dependencies for ".
                                     $this->name." Removing framework. ".
                                     "You can try the --zend option.\n");
        }
      }
    }

    /*******************************
     *  OTHER PULL REQUESTS
     ******************************/
    if (Frameworks::$framework_info[$this->name]
                    ->containsKey("pull_requests")) {
      verbose("Merging some upstream pull requests for ".$this->name."\n",
              Options::$verbose);
      $pull_requests = Frameworks::$framework_info[$this->name]
                                                  ["pull_requests"];
      foreach ($pull_requests as $pr) {
        $dir = $pr["pull_dir"];
        $rep = $pr["pull_repository"];
        $gc = $pr["git_commit"];
        $type = $pr["type"];
        $move_from_dir = null;
        chdir($dir);
        $git_command = "";
        verbose("Pulling code from ".$rep. " and branch/commit ".$gc."\n",
                Options::$verbose);
        if ($type === "pull") {
          $git_command = "git pull --no-edit ".$rep." ".$gc;
        } else if ($type === "submodulemove") {
          $git_command = "git submodule add -b ".$gc." ".$rep;
          $move_from_dir = $pr["move_from_dir"];
        }
        verbose("Pull request command: ".$git_command."\n", Options::$verbose);
        $git_ret = run_install($git_command, $dir,
                               ProxyInformation::$proxies);
        if ($git_ret !== 0) {
          remove_dir_recursive($this->install_root);
          Options::$csv_only ? error()
                             : error("Could not get pull request code for ".
                                     $this->name."!".
                                     " Removing framework!\n");
        }
        if ($move_from_dir !== null) {
          $mv_command = "mv ".$move_from_dir."/* ".$dir;
          verbose("Move command: ".$mv_command."\n", Options::$verbose);
          exec($mv_command);
          verbose("After move, removing: ".$move_from_dir."\n",
                  Options::$verbose);
          remove_dir_recursive($move_from_dir);
        }
        chdir(__DIR__);
      }
    }
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

  public function findTests() {
     // Handle wildcards
    $search_dirs = array();
    foreach($this->test_search_roots as $root) {
      if (strpos($root, "*") !== false || strpos($root, "..") !== false) {
        $globdirs = glob($root, GLOB_ONLYDIR);
        $search_dirs = array_merge($search_dirs, $globdirs);
      } else {
        $search_dirs[] = $root;
      }
    }

    $this->tests = Set{};
    foreach($search_dirs as $root) {
      $this->tests->addAll(find_all_files(PHPUnitPatterns::$test_file_pattern,
                           $root, $this->test_path."/vendor"));
      $this->tests->addAll(find_all_files_containing_text(
                           "extends PHPUnit_Framework_TestCase", $root,
                           $this->test_path."/vendor"));
      // Namespaced case
      $this->tests->addAll(find_all_files_containing_text(
                           "extends \\PHPUnit_Framework_TestCase", $root,
                           $this->test_path."/vendor"));
      // Sometimes a test file extends a class that extends PHPUnit_....
      // Then we have to look at method names.
      $this->tests->addAll(find_all_files_containing_text(
                                            "public function test",
                                            $root, $this->test_path."/vendor"));
    }
    verbose("Found ".count($this->tests)." files that contain tests for ".
            $this->name."...\n", !Options::$csv_only);
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

    /******************************
     *       YII SPECIFIC
     ******************************/
    if ($this->name === "yii") {
      $files = glob($this->install_root.
                    "/tests/assets/*/CAssetManagerTest.php");
      foreach ($files as $file) {
        verbose("Removing $file\n", Options::$verbose);
        unlink($file);
      }
    }
  }

  public function setTestConfiguration() {
    $this->setEnvironmentVariables();
    $this->setTestCommand();
    $this->setTestPattern();
    $this->setTestSearchRoots();
    $this->setTestPath();
    // Assume framework testing is successful until otherwise proven
    $this->success = true;
  }

  public function isInstalled(): bool {
    /****************************************
     *  See if framework is already installed
     *  installed.
     ***************************************/
    $git_head_file =$this->install_root."/.git/HEAD";
    if (!(file_exists($this->install_root))) {
      return false;
    } else if (Options::$force_redownload) {
      verbose("Forced redownloading of ".$this->name."...",
              !Options::$csv_only);
      remove_dir_recursive($this->install_root);
      return false;
    // The commit hash has changed and we need to redownload
    } else if (trim(file_get_contents($git_head_file)) !==
               Frameworks::$framework_info[$this->name]['git_commit']) {
      verbose("Redownloading ".$this->name." because git commit has changed...",
              !Options::$csv_only);
      remove_dir_recursive($this->install_root);
      return false;
    }
    return true;
  }


  private function setTestPath() {
    // 2 possibilities, phpunit.xml and phpunit.xml.dist for configuration
    $phpunit_config_files = Set {'phpunit.xml', 'phpunit.xml.dist'};

    $test_path = null;
    if (Frameworks::$framework_info[$this->name]->contains('test_path')) {
      $this->test_path = Frameworks::$framework_info[$this->name]['test_path'];
    } else {
      $phpunit_config_file_loc = null;
      $phpunit_config_file_loc = find_any_file_recursive($phpunit_config_files,
                                                         $this->install_root,
                                                         true);
      // The test path will be where the phpunit config file is located since
      // that file contains the test directory name. If no config file, error.
      $this->test_path = $phpunit_config_file_loc !== null
                       ? $phpunit_config_file_loc
                       : error("No phpunit test directory found for ".
                               $this->name);
      verbose("Using phpunit xml file in: $phpunit_config_file_loc\n",
              Options::$verbose);
    }
  }

  private function setTestPattern() {
    // Test name pattern can be different depending on the framework,
    // although most follow the default.
    if ($this->name === "pear") {
      $this->test_name_pattern = PHPUnitPatterns::$pear_test_name_pattern;
    } else {
       $this->test_name_pattern = PHPUnitPatterns::$test_name_pattern;
    }
  }

  private function setTestSearchRoots(): void {
    $this->test_search_roots = Frameworks::$framework_info[$this->name]
                                                          ['test_search_roots'];
  }

  private function setTestCommand(): void {
    if (Frameworks::$framework_info[$this->name]
        ->contains('test_run_command')) {
      $this->test_command =
                   Frameworks::$framework_info[$this->name]['test_run_command'];
    } else {
      $this->test_command = get_hhvm_build()." ".__DIR__.
                            "/vendor/bin/phpunit --debug";
    }
    $this->test_command .= " %test%";
    $this->test_command .= " 2>&1";
  }

  private function setEnvironmentVariables(): void {
    if (Frameworks::$framework_info[$this->name]->contains('env_vars')) {
      $this->env_vars = Map {};
      $ev = Frameworks::$framework_info[$this->name]['env_vars'];
      foreach ($ev as $var => $val) {
        $this->env_vars[$var] = $val;
      }
    }
  }

  private function setInstallRoot() {
    $this->install_root = Frameworks::$framework_info[$this->name]
                                                     ['install_root'];
  }

  public static function sort(string $file): bool {
    $results = StableMap {};
    $handle = fopen($file, "r");
    if ($handle) {
      while (!feof($handle)) {
        // trim out newline since StableMap doesn't like them in its keys
        $test = rtrim(fgets($handle), PHP_EOL);
        $status = rtrim(fgets($handle), PHP_EOL);
        $results[$test] = $status;
      }
      if (!ksort($results)) { return false; }
      fclose($handle);
      $contents = "";
      foreach ($results as $test => $status) {
        $contents .= $test.PHP_EOL;
        $contents .= $status.PHP_EOL;
      }
      $contents = remove_last_newline($contents);
      if (file_put_contents($file, $contents) === false) { return false; }
      return true;
    }
    return false;
  }

  public static function removeFinalNewline(string $file): void {
    file_put_contents($file, remove_last_newline(file_get_contents($file)));
  }
}

class SingleTestFile {
  public Framework $framework;
  public string $name;

  private string $test_information = "";
  private string $error_information = "";
  private string $fatal_information = "";
  private string $diff_information = "";
  private string $stat_information = "";

  private array $pipes = null;
  private resource $process = null;

  public function __construct(Framework $f, string $p) {
    $this->framework = $f;
    $this->name = $p;
  }

  public function run(): int {
    chdir($this->framework->test_path);
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
          if ($this->isPreTestWarning($line)) {
            $this->error_information .= "PRETEST WARNING FOR ".
                                        $this->name.PHP_EOL.$line.PHP_EOL;
          }
          continue;
        }
        if ($this->isStop($line)) {
          $post_test = true;
          continue;
        }
        // If we have finished the tests, then we are just printing any error
        // info and getting the final stats
        if ($post_test) {
          $this->printPostTestInfo($line);
        } else if (!$pretest_data) {
          // We have gotten through through the prologue and any blank lines
          // and we should be at tests now.
          if (!$this->analyzeTest($line)) {
            break;
          }
        }
      }

      $ret_val = $this->finalize();
      $this->outputData();

    } else {
      Options::$csv_only ? error()
                         : error("Could not open process to run test ".
                                 $this->name." for framework ".
                                 $this->framework->name);
    }

    chdir(__DIR__);
    return $ret_val;
  }

  private function getLine(): ?string {
    $line = null;
    $s = true;
    // fgets was not working. stream_select seemed ok, but fgets seems to block
    // anyway if only a certain amount of data is available without a newline.
    // Use an fread(), character by character approach.
    while (!feof($this->pipes[1]) && ($s = $this->checkReadStream())) {
      $line .= fgetc($this->pipes[1]);
      if(strstr($line, PHP_EOL) || $line === "") {
        break;
      }
    }
    // We didn't get any chars because checkReadStream failed (timeout)
    if ($s === false) {
      $this->error_information .= "TEST TIMEOUT OCCURRED on test: ".$this->name.
                                   PHP_EOL;
      verbose($this->error_information, !Options::$csv_only);
      return null;
    }
    // Maybe it was the end of the file
    if ($line == null) {
      return null;
    }
    $line = rtrim($line, PHP_EOL);
    $line = remove_color_codes($line);
    return $line;
  }

  private function analyzeTest(string $line): bool {
    $tn_matches = array();
    $match = null;
    $status = "";
    if (preg_match($this->framework->test_name_pattern, $line,
                   $tn_matches) === 1) {
      $match = rtrim($tn_matches[0], PHP_EOL);
      $this->test_information .= $match.PHP_EOL;
      do {
        $status = $this->getLine();
        if ($status === null || $this->checkForFatals($status)) {
          // We have hit a fatal or some nasty assert. Escape now and try to
          // get the results written.
          $status = $status === null ? "UNKNOWN STATUS" : $status;
          $this->test_information .= $status.PHP_EOL;
          $this->fatal_information .= $match.PHP_EOL.$status.PHP_EOL;
          $this->stat_information = $match.PHP_EOL."Fatal".PHP_EOL;
          return false;
        }
      } while (!feof($this->pipes[1]) &&
               preg_match(PHPUnitPatterns::$status_code_pattern,
                          $status) === 0);
      if ($status !== false) {
        $this->processStatus($status, $match);
      }
    } else if ($this->checkForWarnings($line)) {
      // We have a warning after the tests have supposedly started
      // but we really don't have a test to examine.
      // e.g.
      // PHPUnit 3.7.28 by Sebastian Bergmann.
      // The Xdebug extension is not loaded. No code coverage will be generated.
      // HipHop Notice: Use of undefined constant DRIZZLE_CON_NONE
      $this->error_information .= PHP_EOL.$line.PHP_EOL.PHP_EOL;
      return true;
    } else if ($this->checkForFatals($line)) {
      // We have a fatal after the tests have supposedly started
      // but we really don't have a test to examine.
      // e.g.
      // PHPUnit 3.7.28 by Sebastian Bergmann.
      // The Xdebug extension is not loaded. No code coverage will be generated.
      // HipHop Fatal error: Undefined function: mysqli_report
      $this->fatal_information .= PHP_EOL.$line.PHP_EOL.PHP_EOL;
      return false;
    } else {
      // This was something totally unexpected.
      return false;
    }
    return true;
  }

  private function isPreTestWarning(string $line): void {
    if ($this->checkForWarnings($line)) {
      return true;
    }
    return false;
  }

  private function processStatus(string $status, string $match): void {
    // Could be due to a fatal in optimized mode where reasoning is not
    // printed to console (and is only printed in debug mode)
    if ($status === "") {
      $status = "UNKNOWN STATUS";
      $this->fatal_information .= $match.PHP_EOL.$status.PHP_EOL.PHP_EOL;
    } else {
      // In case we had "F 252 / 364 (69 %)" or ".HipHop Warning"
      $status = $status[0];
    }
    $this->test_information .= $status.PHP_EOL;
    if ($this->framework->current_test_statuses !== null &&
        $this->framework->current_test_statuses->containsKey($match)) {
      if ($status === $this->framework->current_test_statuses[$match]) {
        // FIX: posix_isatty(STDOUT) was always returning false, even
        // though can print in color. Check this out later.
        verbose(Colors::GREEN.".".Colors::NONE, !Options::$csv_only);
      } else {
        // We are different than we expected
        $this->framework->success = false;
        // Red if we go from pass to something else
        if ($this->framework->current_test_statuses[$match] === '.') {
          verbose(Colors::RED."F".Colors::NONE, !Options::$csv_only);
        // Green if we go from something else to pass
        } else if ($status === '.') {
          verbose(Colors::GREEN."F".Colors::NONE, !Options::$csv_only);
        // Blue if we go from something "faily" to something "faily"
        // e.g., E to I or F
        } else {
          verbose(Colors::BLUE."F".Colors::NONE, !Options::$csv_only);
        }
        verbose(PHP_EOL."Different status in ".$this->framework->name.
                " for test ".$match." was ".
                $this->framework->current_test_statuses[$match].
                " and now is ".$status.PHP_EOL, !Options::$csv_only);
        $this->diff_information .= "----------------------".PHP_EOL;
        $this->diff_information .= $match.PHP_EOL.PHP_EOL;
        $this->diff_information .= "EXPECTED: ".
                             $this->framework->
                             current_test_statuses[$match].
                             PHP_EOL;
        $this->diff_information .= ">>>>>>>".PHP_EOL;
        $this->diff_information .= "ACTUAL: ".$status.PHP_EOL.PHP_EOL;
      }
    } else {
      // This is either the first time we run the unit tests, and all pass
      // because we are establishing a baseline. OR we have run the tests
      // before, but we are having an issue getting to the actual tests
      // (e.g., yii is one test suite that has behaved this way).
      if ($this->framework->current_test_statuses !== null) {
        $this->framework->success = false;
        verbose(Colors::LIGHTBLUE."F".Colors::NONE, !Options::$csv_only);
        verbose(PHP_EOL."Different status in ".$this->framework->name.
                " for test ".$match.PHP_EOL,!Options::$csv_only);
        $this->diff_information .= "----------------------".PHP_EOL;
        $this->diff_information .= "Problem loading: ".$match.PHP_EOL.PHP_EOL;
      } else {
        verbose(Colors::GRAY.".".Colors::NONE, !Options::$csv_only);
      }
    }
  }

  private function printPostTestInfo(string $line): void {
    while ($line !== null) {
      if (preg_match(PHPUnitPatterns::$tests_ok_skipped_inc_pattern,
                     $line) !== 1 &&
          preg_match(PHPUnitPatterns::$num_errors_failures_pattern,
                     $line) !== 1 &&
          preg_match(PHPUnitPatterns::$failures_header_pattern,
                     $line) !== 1 &&
          preg_match(PHPUnitPatterns::$no_tests_executed_pattern,
                     $line) !== 1 &&
          preg_match(PHPUnitPatterns::$num_skips_inc_pattern,
                     $line) !==1 ) {
        $this->error_information .= $line.PHP_EOL;
      }
      $line = $this->getLine();
    }
    // The last non-null line would have been the real stat information for
    // pass percentage purposes. Take that out of the error string and put in
    // the stat information string instead. Other stat like information may be
    // in the error string as they are part of the test errors (PHPUnit does
    // this).
    $this->error_information = rtrim($this->error_information, PHP_EOL);
    $pieces = explode(PHP_EOL, $this->error_information);
    $this->stat_information = $this->name.PHP_EOL;
    $this->stat_information .= array_pop($pieces).PHP_EOL;
    // There were no errors, just final stats if $pieces is empty
    if (count($pieces) > 0) {
      $this->error_information = implode(PHP_EOL, $pieces).PHP_EOL;
    } else {
      $this->error_information = "";
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
    $test_command = "";
    if ($this->framework->env_vars !== null) {
      foreach($this->framework->env_vars as $var => $val) {
        $test_command .= "export ".$var."=\"".$val."\" && ";
      }
    }
    $test_command .= str_replace("%test%", $this->name,
                                 $this->framework->test_command);
    verbose("Command: ".$test_command."\n", Options::$verbose);

    $descriptorspec = array(
      0 => array("pipe", "r"),
      1 => array("pipe", "w"),
      2 => array("pipe", "w"),
    );

    $env = $_ENV;
    if ($this->framework->env_vars !== null) {
      $env = array_merge($env, $this->framework->env_vars->toArray());
    }
    // $_ENV will passed in by default if the environment var is null
    $this->process = proc_open($test_command, $descriptorspec, $this->pipes,
                               $this->framework->test_path, null);

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
    file_put_contents($this->framework->out_file, $this->test_information,
                      FILE_APPEND);
    file_put_contents($this->framework->errors_file, $this->error_information,
                      FILE_APPEND);
    file_put_contents($this->framework->diff_file, $this->diff_information,
                      FILE_APPEND);
    file_put_contents($this->framework->stats_file, $this->stat_information,
                      FILE_APPEND);
    file_put_contents($this->framework->fatals_file, $this->fatal_information,
                      FILE_APPEND);

  }

  private function checkForFatals(string $status): bool {
    if (strpos($status, 'HipHop Fatal') !== false ||
        strpos($status, 'HHVM Fatal') !== false ||
        strpos($status, 'hhvm Fatal') !== false ||
        strpos($status, "hhvm") !== false) {
      return true;
    }
    return false;
  }

  private function checkForWarnings(string $line): bool {
    if (strpos($line, 'HipHop Warning') !== false ||
        strpos($line, 'HHVM Warning') !== false ||
        strpos($line, 'hhvm Warning') !== false ||
        strpos($line, 'HipHop Notice') !== false ||
        strpos($line, 'HHVM Notice') !== false ||
        strpos($line, 'hhvm notice') !== false) {
      return true;
    }
    return false;
  }
}

function prepare(Vector $framework_names): void {
  Frameworks::init();
  get_unit_testing_infra_dependencies();
  $results_root = __DIR__."/results";

  if (Options::$all) {
    // At this point, $framework_names should be empty if we are in --all mode.
    if (!($framework_names->isEmpty())) {
      error("Do not specify both --all and individual frameworks to run at ".
            "same time.\n");
    }
    // Test all frameworks
    $framework_names = Frameworks::$framework_info->keys();
  } else if (Options::$allexcept) {
    // Run all the frameworks, but the ones we listed.
    $framework_names = Vector::fromItems(
                                array_diff(Frameworks::$framework_info->keys(),
                                           $framework_names));
  } else if (count($framework_names) === 0) {
    error("Specify frameworks to run, use --all or use --allexcept");
  }

  // So it is easier to keep tabs on our progress when running ps or something.
  // Since I get all the tests of a framework by foreaching over the frameworks
  // vector, and then append those tests to a tests vector and then foreach the
  // test vector to bucketize them, this will allow us to basically run the
  // framework tests alphabetically.
  sort($framework_names);
  $frameworks = Vector {};
  foreach ($framework_names as $name) {
    $name = trim(strtolower($name));
    if (Frameworks::$framework_info->containsKey($name)) {
      $framework = new Framework($name);
      $framework->prepareOutputFiles($results_root);
      $frameworks[] = $framework;
    }
  }
  if (count($frameworks) === 0) {
    error("There were no matching frameworks to run");
  }

  /************************
   * Install the frameworks
   ************************/
  fork_buckets($frameworks,
              function($bucket) {return run_install_bucket($bucket);});

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
      error('Issues creating threads for data');
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

function run_install_bucket(array $bucket): int {
  $ret = 0;
  foreach ($bucket as $framework) {
    if (!$framework->isInstalled()) {
      $framework->install();
    }
  }
  return $ret;
}

function run_tests(Vector $frameworks): void {
  if (count($frameworks) === 0) {
    error("No frameworks available on which to run tests");
  }

  /***********************************
   *  Initial preparation
   **********************************/
  $summary_file = tempnam("/tmp", "oss-fw-test-summary");
  $all_tests = Vector {};

  foreach($frameworks as $framework) {
    $framework->clean();
    $framework->setTestConfiguration();
    if (file_exists($framework->expect_file)) {
      $framework->prepareCurrentTestStatuses(
                                        PHPUnitPatterns::$status_code_pattern,
                                        PHPUnitPatterns::$stop_parsing_pattern);
      verbose(Colors::YELLOW.$framework->name.Colors::NONE.": running. ".
              "Comparing against ".count($framework->current_test_statuses).
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
      verbose("Establishing baseline statuses for ".$framework->name.
              " with gray dots...\n", !Options::$csv_only);
    }

    // Get the set of uniquie tests
    $framework->findTests();
    foreach($framework->tests as $test) {
      /**************
       * ZF2 Specific
       **************/
      // These are the two current tests out of ALL tests of ANY framework
      // that seem to be causing deadlock. Removing for now.
      if (strpos($test, "ZendTest/Code/Generator/PropertyGeneratorTest.php") ===
         false &&
         strpos($test, "ZendTest/Code/Generator/ValueGeneratorTest.php") ===
         false) {
        $st = new SingleTestFile($framework, $test);
        $all_tests->add($st);
      }
    }
  }

  /*************************************
   * Run the test suite
   ************************************/
  verbose("Beginning the unit tests.....\n", !Options::$csv_only);
  if (count($all_tests) === 0) {
    error("No tests found to run");
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

  $diff_frameworks = Vector {};
  foreach ($frameworks as $framework) {
    // Remove any final newlines from files. Given the split nature of how the
    // tests run and output to the file, we may have an extra newline.
    // Removing it, makes processing easier later.
    Framework::removeFinalNewline($framework->out_file);
    Framework::removeFinalNewline($framework->diff_file);
    Framework::removeFinalNewline($framework->errors_file);
    Framework::removeFinalNewline($framework->fatals_file);
    Framework::removeFinalNewline($framework->stats_file);
    $pct = $framework->getPassPercentage();
    $encoded_result = json_encode(array($framework->name => $pct));
    if (!(file_exists($summary_file))) {
      file_put_contents($summary_file, $encoded_result);
    } else {
      $file_data = file_get_contents($summary_file);
      $decoded_results = json_decode($file_data, true);
      $decoded_results[$framework->name] = $pct;
      file_put_contents($summary_file, json_encode($decoded_results));
    }

    // If the first baseline run, make both the same. Otherwise, see if we have
    // a diff file. If not, then all is good. If not, thumbs down because there
    // was a difference between what we ran and what we expected.
    if (!file_exists($framework->expect_file)) {
      copy($framework->out_file, $framework->expect_file);
      Framework::sort($framework->expect_file);
    } else if (filesize($framework->diff_file) > 0) {
      $diff_frameworks[] = $framework;
      $framework->success = false;
    }
  }

  if ($framework->success) {
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
  // Install composer.phar
  if (!(file_exists(__DIR__."/composer.phar"))) {
    verbose("Getting composer.phar....\n", !Options::$csv_only);
    $comp_url = "http://getcomposer.org/composer.phar";
    $get_composer_command = "wget ".$comp_url." -P ".__DIR__." 2>&1";
    $ret = run_install($get_composer_command, __DIR__,
                       ProxyInformation::$proxies);
    if ($ret !== 0) {
      error("Could not download composer. Script stopping\n");
    }
  }

  // Install phpunit from composer.json located in __DIR__
  $phpunit_binary = __DIR__."/vendor/bin/phpunit";
  if (!(file_exists($phpunit_binary))) {
    verbose("\nDownloading PHPUnit in order to run tests. There may be an ".
            "output delay while the download begins.\n", !Options::$csv_only);
    // Use the timeout to avoid curl SlowTimer timeouts and problems
    $phpunit_install_command = get_hhvm_build()." ".
                               "-v ResourceLimit.SocketDefaultTimeout=30 ".
                               __DIR__.
                               "/composer.phar install --dev --verbose 2>&1";
    $ret = run_install($phpunit_install_command, __DIR__,
                       ProxyInformation::$proxies);
    if ($ret !== 0) {
      error("Could not install PHPUnit. Script stopping\n");
    }
  }

}

// This will run processes that will get the test infra dependencies
// (e.g. PHPUnit), frameworks and framework dependencies.
function run_install(string $proc, string $path, ?Map $env): ?int
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
        verbose(".", !Options::$csv_only);
        $start_time = microtime(true);
      }
    }
    fclose($pipes[1]);
    $ret = proc_close($process);
    return $ret;
  }
  return null;
}

function print_diffs(Framework $framework): void {
  $diff = $framework->diff_file;
  // The file may not exist or the file may not have anything in it
  // since there is no diff (i.e., all tests for that particular
  // framework ran as expected). Either way, don't print anything
  // out for those cases.
  if (file_exists($diff) &&
     ($contents = file_get_contents($diff)) !== "") {
    print PHP_EOL."********* ".strtoupper($framework->name).
          " **********".PHP_EOL;
    print $contents;
    print "To run tests for ".strtoupper($framework->name).
          " use this command: \n";
    $dir = $framework->test_path;
    $command = "cd ".$dir." && ";
    $command .= $framework->test_command;
    print $command." [TEST NAME]".PHP_EOL;
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
      print "\nALL TESTS COMPLETE!\n";
      print "SUMMARY:\n";
      foreach ($decoded_results as $key => $value) {
        print $key."=".$value.PHP_EOL;
      }
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

function oss_test_option_map(): OptionInfoMap {
  return Map {
    'help'                => Pair {'h', "Print help message"},
    'all'                 => Pair {'a',  "Run tests of all frameworks. The ".
                                        "frameworks to be run are hardcoded ".
                                        "in a Map in this code."},
    'allexcept'           => Pair {'e', "Run all tests of all frameworks ".
                                        "except for the ones listed. The ".
                                        "tests must be at the end of the ".
                                        "command argument list."},
    'timeout:'            => Pair {'', "Optional - The maximum amount of ".
                                        "time, in secs, to allow a individual".
                                        "test to run. Default is 60 seconds."},
    'verbose'             => Pair {'v', "Optional - For a lot of messages ".
                                        "about what is going on."},
    'zend:'               => Pair {'', "Optional - Try to use zend if ".
                                        "retrieving dependencies with hhvm ".
                                        "fails. Currently, zend must be ".
                                        "installed and the path to the zend ".
                                        "binary specified."},
    'redownload'          => Pair {'', "Forces a redownload of the framework ".
                                        "code and dependencies."},
    'record'              => Pair {'', "Forces a new expect file for the ".
                                        "framework test suite"},
    'csv'                 => Pair {'', "Just create the machine readable ".
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

function find_all_files(string $pattern, string $root_dir,
                        string $exclude = null): Set {
  $files = Set {};
  $dit = new RecursiveDirectoryIterator($root_dir,
                                        RecursiveDirectoryIterator::SKIP_DOTS);
  $rit = new RecursiveIteratorIterator($dit);
  $sit = new SortedIterator($rit);
  foreach ($sit as $fileinfo) {
    if (preg_match($pattern, $fileinfo->getFileName()) === 1 &&
        strpos($fileinfo->getPathName(), $exclude) === false) {
      $files[] = $fileinfo->getPathName();
    }
  }

  return $files;
}

function find_all_files_containing_text(string $text,
                                        string $root_dir,
                                        string $exclude = null): Set {
  $files = Set {};
  $dit = new RecursiveDirectoryIterator($root_dir,
                                        RecursiveDirectoryIterator::SKIP_DOTS);
  $rit = new RecursiveIteratorIterator($dit);
  $sit = new SortedIterator($rit);
  foreach ($sit as $fileinfo) {
    if (strpos(file_get_contents($fileinfo->getPathName()), $text) !== false &&
        strpos($fileinfo->getPathName(), $exclude) === false) {
      $files[] = $fileinfo->getPathName();
    }
  }

  return $files;
}

function get_hhvm_build(bool $with_jit = true, bool $use_php = false): string {
  $fbcode_root_dir = __DIR__.'/../../..';
  $oss_root_dir = __DIR__.'../..';
  $build = "";
  // See if we are using an internal development build
  if ((file_exists($fbcode_root_dir."/_bin"))) {
    $build .= $fbcode_root_dir;
    if (!$use_php) {
      $build .= "/_bin/hphp/hhvm/hhvm -v Eval.EnableZendCompat=true";
    } else {
      $build .= "/_bin/hphp/hhvm/php";
    }
  } else if (file_exists($oss_root_dir."/".
                         idx($_ENV, 'FBMAKE_BIN_ROOT', '_bin'))) {
    // Maybe we are in OSS land trying this script
    $build .= $oss_root_dir."/".idx($_ENV, 'FBMAKE_BIN_ROOT', '_bin');
    if (!$use_php) {
      $build .= "/hhvm -v Eval.EnableZendCompat=true";
    } else {
      $build .= "/php";
    }
  } else {
    error("HHVM build doesn't exist. Did you build yet?");
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
  return $build;
}

function idx(array $array, mixed $key, mixed $default = null): mixed {
  return isset($array[$key]) ? $array[$key] : $default;
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

function remove_color_codes(string $line): string {
  return preg_replace(PHPUnitPatterns::$color_escape_code_pattern,
                      "", $line);
}

function remove_last_newline(string $text): string {
  $s = substr($text, -1);
  return $s === PHP_EOL ? substr($text, 0, -1) : $text;
}

function main(array $argv): void {
  $options = parse_options(oss_test_option_map());
  if ($options->containsKey('help')) {
    return help();
  }
  // Parse other possible options out in run()
  $framework_names = Options::parse($options, $argv);
  $frameworks = prepare($framework_names);
  run_tests($frameworks);
}

main($argv);
