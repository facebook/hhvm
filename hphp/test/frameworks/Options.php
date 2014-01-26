<?hh
require_once __DIR__.'/spyc/Spyc.php';
require_once __DIR__.'/utils.php';
require_once __DIR__.'/ProxyInformation.php';
require_once __DIR__.'/../../../hphp/tools/command_line_lib.php';

class Options {
  public static string $frameworks_root;
  // seconds to run any individual test for any framework
  public static int $timeout = 90;
  public static bool $verbose = false;
  public static bool $csv_only = false;
  public static bool $csv_header = false;
  public static bool $force_redownload = false;
  public static bool $get_latest_framework_code = false;
  public static bool $generate_new_expect_file = false;
  public static string $zend_path = null;
  public static bool $all = false;
  public static bool $allexcept = false;
  public static bool $test_by_single_test = false;
  public static string $results_root;
  public static string $script_errors_file;
  public static array $framework_info;
  public static array $original_framework_info;

  public static function parse(OptionInfoMap $options, array $argv): Vector {
    self::$frameworks_root = __DIR__.'/framework_downloads';
    self::$framework_info = Spyc::YAMLLoad(__DIR__."/frameworks.yaml");
    self::$original_framework_info = self::$framework_info;
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

    if ($options->containsKey('redownload') &&
        $options->containsKey('latest')) {
      error_and_exit("Cannot use --redownload and --latest together");
    } else if ($options->containsKey('redownload') &&
               $options->containsKey('latest-record')) {
      error_and_exit("Cannot use --redownload and --latest-record together");
    } else if ($options->containsKey('redownload')) {
      self::$force_redownload = true;
      $framework_names->removeKey(0);
    } else if ($options->containsKey('latest')) {
      self::$get_latest_framework_code = true;
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('record')) {
      self::$generate_new_expect_file = true;
      $framework_names->removeKey(0);
    }

    // Probably bad practice to have --latest --record --latest-record, but it
    // is not a contradiction. If you have those, we are just double setting
    // variables to true.
    if ($options->containsKey('latest-record')) {
      self::$get_latest_framework_code = true;
      self::$generate_new_expect_file = true;
      $framework_names->removeKey(0);
    }

    // This will return just the name of the frameworks passed in, if any left
    // (e.g. --all may have been passed, in which case the Vector will be
    // empty)
    return $framework_names;
  }
}
