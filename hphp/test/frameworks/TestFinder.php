<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
$FBCODE_ROOT = __DIR__.'/../../..';
include_once $FBCODE_ROOT.'/hphp/tools/command_line_lib.php';
require_once __DIR__.'/SortedIterator.php';
require_once __DIR__.'/utils.php';
require_once __DIR__.'/TestFindModes.php';

// For reflection in finding tests
include_once __DIR__."/vendor/phpunit/phpunit/PHPUnit/Autoload.php";

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

  public function findTestMethods(): void {
    $current_classes = get_declared_classes();
    $tests = "";
    include_once $this->bootstrap_file;
    foreach ($this->test_files as $tf) {
      if (strpos($tf, ".phpt") !== false) {
        $tests .= $tf.PHP_EOL;
        continue;
      }
      include_once $tf;
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
    $test_files = "";
    foreach ($this->test_files as $test_file) {
      $test_files .= $test_file.PHP_EOL;
    }
    file_put_contents($this->tests_file, $test_files);
  }

  public function findTestMethodsViaTokens(): void {
    $tests = "";
    foreach ($this->test_files as $test_file) {
      $php_code = file_get_contents($test_file);
      $tokens = token_get_all($php_code);
      $count = count($tokens);
      for ($i = 6; $i < $count; $i++) {
        if ($tokens[$i - 6][0] === T_CLASS &&       // class keyword
            $tokens[$i - 5][0] === T_WHITESPACE &&
            $tokens[$i - 4][0] === T_STRING &&      // class name
            $tokens[$i - 3][0] === T_WHITESPACE &&
            $tokens[$i - 2][0] === T_EXTENDS &&      // extends keyword
            $tokens[$i - 1][0] === T_WHITESPACE &&
            $tokens[$i - 0][0] === T_STRING &&
            (strpos($tokens[$i - 0][1], "TestCase") !== false || // parent name
             strpos($tokens[$i - 0][1], "test_case") !== false)) {
          $class_name = $tokens[$i - 4][1];
        }
        if ($tokens[$i - 2][0] === T_FUNCTION &&
            $tokens[$i - 1][0] === T_WHITESPACE &&
            $tokens[$i][0] === T_STRING &&
            strpos($tokens[$i][1], "test") === 0) {
              $tests .= $class_name."::".$tokens[$i][1].PHP_EOL;
        }
      }
    }
    file_put_contents($this->tests_file, $tests);
  }

  public function findTestFiles(): void {
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
    if ($test_suite->count() === 0) {
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
        if ($dir->attributes()->count() === 0) {
          $pattern = $this->test_file_pattern;
        } else {
          $pattern = "/".$dir->attributes()->suffix."/";
        }
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

function help(): void {
  display_help("Finds the tests for the given framework",
               oss_test_option_map());
}

function oss_test_option_map(): OptionInfoMap {
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

function main(array $argv): void {
  $options = parse_options(oss_test_option_map());
  if ($options->containsKey('help')) {
    help();
    exit(0);
  }
  try {
    $fn = $options['framework-name'];
    $tf = $options['tests-file'];
    $tff = $options['test-files-file'];
    $tp = $options['test-path'];
    $tfp = $options['test-file-pattern'];
    $cf = $options['config-file'];
    $bf = $options->containsKey('bootstrap-file')
          ? $options['bootstrap-file']
          : null;
    $mode = $options['test-find-mode'];
  } catch (Exception $e) {
    help();
    echo "Provide required command line arguments!\n";
    exit(-1);
   }
  $tf = new TestFinder($fn, $tf, $tff, $tp, $tfp, $cf, $bf);
  // Mediawiki and others are clowntown when it comes to autoloading stuff
  // for reflection. Or I am a clown. Either way, workaround it.
  // May try spl_autoload_register() to workaround some clowniness, if possible.
  if ($mode === TestFindModes::TOKEN) {
    $tf->findTestMethodsViaTokens();
  } else if ($mode === TestFindModes::PHPT) {
    $tf->findTestsPHPT();
  } else {
    $tf->findTestMethods();
  }
  exit(0);
}


main($argv);
