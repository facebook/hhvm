<?hh
require_once __DIR__.'/../../../hphp/tools/command_line_lib.php';
require_once __DIR__.'/SortedIterator.php';
require_once __DIR__.'/utils.php';
require_once __DIR__.'/TestFindModes.php';

class TestFinder {
  private Set $test_files;

  public function __construct(private string $framework_name,
                              private string $tests_file,
                              private string $test_files_file,
                              private string $test_path,
                              private string $test_file_pattern,
                              private string $config_file,
                              private ?string $bootstrap_file) {
    $this->findTestFiles();
  }

  // BUG FIX NEEDED: Due to some recent updates to Reflection, this default
  // method for finding tests in frameworks is failing for certain
  // frameworks. The two failing frameworks are Monolog and ZF2. Luckily,
  // I was able to move those tests to use TestFindModes::TOKEN. But
  // there is a problem, either with this code (that worked before the
  // Reflection changes) or Reflection itself.
  // See git log 63e752f4f0bd141d4e401e7056b4850e7b9a4406 and git log
  // faec990edc3e3e8f3b491070b0e8cd90e9df7a4d for the addition of the
  // new ext_reflection-classes.php class.
  public function findTestMethods(): void {
    // For reflection in finding tests
    // Since PHPUnit 3.8, Autoload.php has gone away.
    // And the main source directory 'PHPUnit' was changed to 'src'.
    // So check for both and do the right thing
    if (file_exists(__DIR__."/vendor/phpunit/phpunit/src/")) {
      // For 3.8+
      require_once __DIR__."/vendor/autoload.php";
    } else if (file_exists(__DIR__."/vendor/phpunit/phpunit/PHPUnit/")) {
      // For 3.7 and below
      require_once __DIR__."/vendor/phpunit/phpunit/PHPUnit/Autoload.php";
    } else {
      // Fallback to token based test finding
      findTestMethodsViaToken();
      return;
    }

    if ($this->bootstrap_file !== null) {
      require_once $this->bootstrap_file;
    }

    $current_classes = get_declared_classes();
    $tests = "";
    foreach ($this->test_files as $tf) {
      if (strpos($tf, ".phpt") !== false) {
        $tests .= $tf.PHP_EOL;
        continue;
      }
      require_once $tf;
      // New classes will be brought in by the include; get the difference
      // between what was currently loaded.
      $file_classes = array_diff(get_declared_classes(), $current_classes);
      foreach ($file_classes as $class_name) {
        $class = new ReflectionClass($class_name);
        if ($class->isSubclassOf("PHPUnit_Framework_TestCase")) {
          $class_methods = $class->getMethods(ReflectionMethod::IS_PUBLIC);
          foreach($class_methods as $method) {
            if (strpos($method->getShortName(), "test") === 0 ||
                strpos($method->getDocComment(), "@test") !== false) {
              $tests .= $class->getName()."::".$method->getShortName().PHP_EOL;
            }
          }
        }
      }
      // We don't want to see the current classes again in the next
      // iteration of the loop. They will now be removed with the array_diff
      $current_classes = get_declared_classes();
    }
    file_put_contents($this->tests_file, $tests);
  }

  public function findTestsPHPT(): void {
    file_put_contents(
      $this->tests_file,
      implode(PHP_EOL, $this->test_files->toArray())
    );
  }

  public function findTestMethodsViaTokens(): void {
    $tests = "";
    foreach ($this->test_files as $test_file) {
      $php_code = file_get_contents($test_file);
      $tokens = token_get_all($php_code);
      $count = count($tokens);
      $nspace = "";
      $class_name = "";
      // Get the namespace the class is in, if any
      for ($i = 1; $i < $count; $i++) {
        if ($tokens[$i - 1][0] === T_NAMESPACE && // namespace keyword
            $tokens[$i][0] === T_WHITESPACE) {
          $i++;
          // Get the full namespace until we hit a ;
          while ($tokens[$i][0] !== ";") {
            $nspace .= $tokens[$i][1];
            $i++;
          }
          break;
        }
      }
      // Get the namespace qualified (if any) class name
      for ($i = 6; $i < $count; $i++) {
        if ($tokens[$i - 6][0] === T_CLASS &&       // class keyword
            $tokens[$i - 5][0] === T_WHITESPACE &&
            $tokens[$i - 4][0] === T_STRING &&      // class name
            $tokens[$i - 3][0] === T_WHITESPACE &&
            $tokens[$i - 2][0] === T_EXTENDS &&      // extends keyword
            $tokens[$i - 1][0] === T_WHITESPACE) {
          $classpos = $i - 4;
          // parent could be non-namespaced or be namespaced
          // So get through all T_NS_SEPARATOR and T_STRING
          // Last T_STRING before T_WHITESPACE will be parent name
          while ($tokens[$i][0] !== T_WHITESPACE) { $i++;}
          if (strpos($tokens[$i - 1][1], "TestCase") !== false || // parent name
              strpos($tokens[$i - 1][1], "test_case") !== false) {
            $class_name = $tokens[$classpos][1];
          }
          break;
        }
      }
      // Get all the test functions in the class
      for ($i = 2; $i < $count; $i++) {
        if ($tokens[$i - 2][0] === T_FUNCTION &&
            $tokens[$i - 1][0] === T_WHITESPACE &&
            $tokens[$i][0] === T_STRING &&
            strpos($tokens[$i][1], "test") === 0) {
          if ($nspace !== "") {
            $tests .= $nspace."\\";
          }
          $tests .= $class_name."::".$tokens[$i][1].PHP_EOL;
        }
      }
    }

    file_put_contents($this->tests_file, $tests);
  }

  private function findTestFiles(): void {
    $this->test_files = Set {};
    $exclude_pattern = "/\.disabled\.hhvm/";
    $exclude_dirs = Set {};
    $config_xml = simplexml_load_file($this->config_file);
    // The bootstrap file will be defined as a top level attribute to the
    // <phpunit> element and the path to it will be relative to the location
    // of the config file
    if ($this->bootstrap_file === null &&
        $config_xml->attributes()->bootstrap !== null) {
      $this->bootstrap_file = dirname($this->config_file)."/".
                              $config_xml->attributes()->bootstrap;
    }
    // Some framework phpunit.xml files do not have a <testsuites><testsuite>
    // element, just a single <testsuite> element
    $test_suite = $config_xml->testsuites->testsuite;
    if ($test_suite === null || $test_suite->count() === 0) {
      $test_suite = $config_xml->testsuite;
    }
    foreach ($test_suite as $suite) {
      foreach($suite->exclude as $exclude) {
        $exclude_dirs->add($this->test_path."/".$exclude);
      }
      foreach($suite->file as $file) {
        if (file_exists($this->config_file."/".$file)) {
          $this->test_files->add($file);
        }
      }
      foreach($suite->directory as $dir) {
        $search_dirs = null;
        // If config doesn't provide a test suffix, assume the default
        $pattern = $dir->attributes()->count() === 0
          ? $this->test_file_pattern
          : "/".$dir->attributes()->suffix."/";
        $search_path = $this->test_path."/".$dir;
        // Gotta make sure these dirs don't contain wildcards (Looking at you
        // Symfony)
        if (strpos($search_path, "*") !== false ||
            strpos($search_path, "..") !== false) {
          $search_dirs = glob($search_path, GLOB_ONLYDIR);
        }
        if ($search_dirs !== null) {
          foreach($search_dirs as $sd) {
            $this->test_files->addAll(find_all_files($pattern,
                                                     $sd,
                                                     $exclude_pattern,
                                                     $exclude_dirs));
          }
        } else {
          $this->test_files->addAll(find_all_files($pattern,
                                                   $this->test_path."/".$dir,
                                                   $exclude_pattern,
                                                   $exclude_dirs));
        }
      }
    }
    foreach($this->test_files as $file) {
      file_put_contents($this->test_files_file, $file.PHP_EOL, FILE_APPEND);
    }
  }
}

function tf_help(): void {
  display_help("Finds the tests for the given framework",
               test_finder_option_map());
}

function test_finder_option_map(): OptionInfoMap {
  return Map {
    'help'                => Pair {'h', "Print help message."},
    'framework-name:'     => Pair {'',  "The framework name."},
    'config-file:'        => Pair {'',  "The full path to the framework xml ".
                                        "configuration file."},
    'bootstrap-file:'     => Pair {'',  "The full path to the bootstrap file, ".
                                        "if it exists. This is optional."},
    'tests-file:'         => Pair {'',  "The full path to the file where the ".
                                        "framework tests will be output."},
    'test-files-file:'    => Pair {'',  "The full path to the file where the ".
                                        "framework test files will be output."},
    'test-path:'          => Pair {'',  "The full path to the directory where ".
                                        "the testing will begin."},
    'test-file-pattern:'  => Pair {'',  "The regex pattern which test files ".
                                        "are named for this framework. E.g., ".
                                        "xxxxTest.php or xxxxx.phpt"},
    'test-find-mode:'     => Pair {'',  "The mode to use in order to find ".
                                        "the tests for this framework. Your ".
                                        "options are 'reflection' (the ".
                                        "default), 'token' or 'phpt'."},
  };
}

function tf_main(array $argv): void {
  $options = parse_options(test_finder_option_map());
  if ($options->containsKey('help')) {
    tf_help();
    exit(0);
  }
  try {
    $fn = (string) $options['framework-name'];
    $tf = (string) $options['tests-file'];
    $tff = (string) $options['test-files-file'];
    $tp = (string) $options['test-path'];
    $tfp = (string) $options['test-file-pattern'];
    $cf = (string) $options['config-file'];
    $bf = $options->containsKey('bootstrap-file')
          ? (string) $options['bootstrap-file']
          : null;
    $mode = (string) $options['test-find-mode'];
  } catch (Exception $e) {
    tf_help();
    echo "Provide required command line arguments!\n";
    exit(-1);
    throw $e; // unreachable, but makes typechecker happy - #2916
  }
  $tf = new TestFinder($fn, $tf, $tff, $tp, $tfp, $cf, $bf);
  // Mediawiki and others are clowntown when it comes to autoloading stuff
  // for reflection. Or I am a clown. Either way, workaround it.
  // May try spl_autoload_register() to workaround some clowniness, if possible.
  switch ($mode) {
    case TestFindModes::TOKEN:
      $tf->findTestMethodsViaTokens();
      break;
    case TestFindModes::PHPT:
      $tf->findTestsPHPT();
      break;
    default:
      $tf->findTestMethods();
  }
  exit(0);
}


tf_main($argv);
