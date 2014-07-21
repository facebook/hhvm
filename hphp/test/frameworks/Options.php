<?hh
require_once __DIR__.'/spyc/Spyc.php';
require_once __DIR__.'/utils.php';
require_once __DIR__.'/ProxyInformation.php';
require_once __DIR__.'/../../../hphp/tools/command_line_lib.php';

class OutputFormat {
  const int HUMAN = 1;
  const int HUMAN_VERBOSE = 2;
  const int CSV = 3;
  // A stream of JSON objects in a format that Facebook's test systems
  // understand. If you're a Facebook employee, take a look at the
  // JsonTestRunner class for the format specification.
  const int FBMAKE = 4;
};

class Options {
  public static string $frameworks_root = __DIR__.'/framework_downloads';
  // seconds to run any individual test for any framework
  public static int $timeout = 90;
  public static int $output_format = OutputFormat::HUMAN;
  public static bool $csv_header = false;
  public static bool $force_redownload = false;
  public static bool $get_latest_framework_code = false;
  public static bool $generate_new_expect_file = false;
  public static bool $include_flakey = false;
  public static ?string $php_path = null;
  public static bool $all = false;
  public static bool $allexcept = false;
  public static bool $test_by_single_test = false;
  public static string $results_root = __DIR__.'/results';
  public static string $script_errors_file = __DIR__.'/results/_script.errors';
  public static string $generated_ini_file = __DIR__.'/.generated.php.ini';
  public static array $framework_info = [];
  public static array $original_framework_info = [];
  public static int $num_threads = -1;
  public static bool $as_phpunit = false;
  public static ?string $toran_proxy = null;

  public static function parse(OptionMap $options, array $argv): Vector {
    $ini_settings = Map { };

    self::$framework_info = Spyc::YAMLLoad(__DIR__."/frameworks.yaml");
    self::$original_framework_info = self::$framework_info;
    // Put any script error to a file when we are in a mode like --csv and
    // want to control what gets printed to something like STDOUT.
    unlink(self::$script_errors_file);

    // Don't use $argv[0] which just contains the program to run
    $framework_names = new Vector(array_slice($argv, 1));

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

    if ($options->containsKey('flakey')) {
      if ($options->containsKey('record')) {
        error_and_exit('Can not use --flakey and --record together');
      }
      self::$include_flakey = true;
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('csv')) {
      if ($options->containsKey('verbose')) {
        error_and_exit("Cannot use --csv and --verbose together");
      }
      if ($options->containsKey('fbmake')) {
        error_and_exit("Cannot use --csv and --fbmake together");
      }
      self::$output_format = OutputFormat::CSV;
      // $tests[0] may not even be "summary", but it doesn't matter, we are
      // just trying to make the count right for $frameworks
      $framework_names->removeKey(0);
    } else if ($options->containsKey('verbose')) {
      if ($options->containsKey('fbmake')) {
        error_and_exit("Cannot use --fbmake and --verbose together");
      }
      self::$output_format = OutputFormat::HUMAN_VERBOSE;
      $framework_names->removeKey(0);
    } else if ($options->containsKey('fbmake')) {
      self::$output_format = OutputFormat::FBMAKE;
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('csvheader')) {
      if (!$options->containsKey('csv')) {
        error_and_exit("Must have --csv to use --csvheader");
      }
      self::$csv_header = true;
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('as-phpunit')) {
      if ($options->containsKey('by-single-test') ||
          $options->containsKey('by-file') ||
          $options->containsKey('numthreads')) {
        error_and_exit("Cannot specify as-phpunit with by-file ".
                       "by-single-test, or numthreads");
      }
      self::$as_phpunit = true;
      $framework_names->removeKey(0);
    } else if ($options->contains('by-single-test')) {
      if ($options->containsKey('by-file')) {
        // Can't run framework tests both by file and single test
        error_and_exit("Cannot specify both by-file and by-single-test");
      }
      self::$test_by_single_test = true;
      $framework_names->removeKey(0);
    } else if ($options->contains('by-file')) {
      // Nothing to set here since this is the default, but remove the key
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('numthreads')) {
      self::$num_threads = (int) $options['numthreads'];
      if (self::$num_threads < 1) {
        self::$num_threads = 1;
      }
      // Remove numthreads option and its value from the $framework_names vector
      $framework_names->removeKey(0);
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('timeout')) {
      self::$timeout = (int) $options['timeout'];
      // Remove timeout option and its value from the $framework_names vector
      $framework_names->removeKey(0);
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('with-php')) {
       self::$php_path = (string) $options['with-php'];
      $framework_names->removeKey(0);
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('isolate')) {
      $ini_settings['hhvm.jit_enable_rename_function'] = true;
      $ini_settings['auto_prepend_file'] = __DIR__.'/Isolation.php';
      $framework_names->removeKey(0);
    }

    if ($options->containsKey('redownload')) {
      if ($options->containsKey('latest')) {
        error_and_exit("Cannot use --redownload and --latest together");
      }
      if ($options->containsKey('latest-record')) {
        error_and_exit("Cannot use --redownload and --latest-record together");
      }
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

    if ($options->containsKey('toran-proxy')) {
      self::$toran_proxy = ((string) $options['toran-proxy']) ?: null;
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

    $ini_string = '';
    foreach ($ini_settings as $k => $v) {
      $ini_string .= sprintf("%s=%s\n", $k, var_export($v, true));
    }
    verbose("Generated INI string: \n".$ini_string);
    file_put_contents(self::$generated_ini_file, $ini_string);

    verbose("Script running...Be patient as some frameworks take a while with ".
            "a debug build of HHVM\n");

    if (ProxyInformation::is_proxy_required()) {
      verbose("Looks like proxy may be required. Setting to default FB proxy ".
              "values. Please change Map in ProxyInformation.php to correct ".
              "values, if necessary.\n");
    }

    // This will return just the name of the frameworks passed in, if any left
    // (e.g. --all may have been passed, in which case the Vector will be
    // empty)
    return $framework_names;
  }

  // Will return a string (e.g. for test path) or
  // an array (e.g., for blacklisted tests)
  //
  // Currently untyped for convenience. Prefer splitting into two methods to
  // calling it mixed.
  public static function getFrameworkInfo(string $framework, string $key) {
    return array_key_exists($key, self::$framework_info[$framework])
      ? self::$framework_info[$framework][$key]
      : null;
  }
}
