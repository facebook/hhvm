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
            $shortname = $method->getShortName();
            $doccomment = $method->getDocComment();
            if (($shortname is string && strpos($shortname, "test") === 0) ||
                ($doccomment is string && strpos($doccomment, "@test") !== false)) {
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
    $tests = vec[];
    foreach ($this->test_files as $test_file) {
      $json = HH\ffp_parse_string(file_get_contents($test_file));
      $methods = HH\ExperimentalParserUtils\find_test_methods($json);
      foreach ($methods as list($parent, $class, $method)) {
        $parent_no_ns = end(&explode('\\', $parent));
        if ((strpos($parent_no_ns, "TestCase") !== false ||
             strpos($parent_no_ns, "test_case") !== false) &&
            strpos($method, "test") === 0) {
          $tests[] = $class . '::' . $method;
        }
      }
    }
    $tests[] = '';
    $tests = implode(PHP_EOL, $tests);
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
