<?hh
/**
* Run the test suites in various configurations.
*/

const TIMEOUT_SECONDS = 300;

function is_testing_dso_extension() {
  // detecting if we're running outside of the hhvm codebase.
  return !is_file(__DIR__ . "/../../hphp/test/run");
}

function get_expect_file_and_type($test, $options) {
  // .typechecker files are for typechecker (hh_server --check) test runs.
  $types = null;
  if (isset($options['typechecker'])) {
    $types = array('typechecker.expect', 'typechecker.expectf');
  } else {
    $types = array('expect', 'hhvm.expect', 'expectf', 'hhvm.expectf',
                   'expectregex');
  }
  if (isset($options['repo'])) {
    foreach ($types as $type) {
      $fname = "$test.$type-repo";
      if (file_exists($fname)) {
        return array($fname, $type);
      }
    }
  }
  foreach ($types as $type) {
    $fname = "$test.$type";
    if (file_exists($fname)) {
      return array($fname, $type);
    }
  }
  return array(null, null);
}

function multi_request_modes() {
  return array('relocate',
               'retranslate-all',
               'recycle-tc',
               'jit-serialize',
               'cli-server');
}

function has_multi_request_mode($options) {
  foreach (multi_request_modes() as $option) {
    if (isset($options[$option])) return true;
  }
  return false;
}

function jit_serialize_option($cmd, $test, $options, $serialize) {
  $serialized = "$test.repo/jit.dump";
  $cmds = explode(' -- ', $cmd, 2);
  $cmds[0] .=
    ' --count=' . ($serialize ? $options['jit-serialize'] : 1) .
    " -vEval.JitSerdesFile=" . $serialized .
    " -vEval.JitSerdesMode=" . ($serialize ? 'Serialize' : 'DeserializeOrFail');
  if (isset($options['jitsample']) && $serialize) {
    $cmds[0] .= ' -vDeploymentId="' . $options['jitsample'] . '-serialize"';
  }
  return implode(' -- ', $cmds);
}

function usage() {
  global $argv;
  return "usage: $argv[0] [-m jit|interp] [-r] <test/directories>";
}

function help() {
  global $argv;
  $ztestexample = 'test/zend/good/*/*z*.php'; // sep. for syntax highlighting.
  $help = <<<EOT


This is the hhvm test-suite runner.  For more detailed documentation,
see hphp/test/README.md.

The test argument may be a path to a php test file, a directory name, or
one of a few pre-defined suite names that this script knows about.

If you work with hhvm a lot, you might consider a bash alias:

   alias ht="path/to/hphp/test/run"

Examples:

  # Quick tests in JIT mode:
  % $argv[0] test/quick

  # Slow tests in interp mode:
  % $argv[0] -m interp test/slow

  # PHP specificaion tests in JIT mode:
  % $argv[0] test/spec

  # Slow closure tests in JIT mode:
  % $argv[0] test/slow/closure

  # Slow closure tests in JIT mode with RepoAuthoritative:
  % $argv[0] -r test/slow/closure

  # Slow array tests, in RepoAuthoritative:
  % $argv[0] -r test/slow/array

  # Zend tests with a "z" in their name:
  % $argv[0] $ztestexample

  # Quick tests in JIT mode with some extra runtime options:
  % $argv[0] test/quick -a '-vEval.JitMaxTranslations=120 -vEval.HHIRRefcountOpts=0'

  # All quick tests except debugger
  % $argv[0] -e debugger test/quick

  # All tests except those containing a string of 3 digits
  % $argv[0] -E '/\d{3}/' all

  # All tests whose name containing pdo_mysql
  % $argv[0] -i pdo_mysql -m jit -r zend

  # Print all the standard tests
  % $argv[0] --list-tests

  # Use a specific HHVM binary
  % $argv[0] -b ~/code/hhvm/hphp/hhvm/hhvm
  % $argv[0] --hhvm-binary-path ~/code/hhvm/hphp/hhvm/hhvm

  # Use live relocation to run tests in the same thread. e.g, 6 times in the same
  # thread, where the 3 specifies a random relocation for the 3rd request and the
  # test is run 3 * 2 times.
  % $argv[0] --relocate 3 test/quick/silencer.php

  # Use retranslate all.  Run the test n times, then run retranslate all, then
  # run the test n more on the new code.
  % $argv[0] --retranslate-all 2 quick

  # Use jit-serialize.  Run the test n times, then run retranslate all,
  # serialize the state, and then restart hhvm, load the serialized state and
  # run retranslate-all before starting the test.
  % $argv[0] --jit-serialize  2 -r quick

  # Run the Hack typechecker against quick typechecker.expect[f] files
  # Could explcitly use quick here too
  # $argv[0] --typechecker

  # Run the Hack typechecker against typechecker.expect[f] files in the slow
  # directory
  # $argv[0] --typechecker slow

  # Run the Hack typechecker against the typechecker.expect[f] file in this test
  # $argv[0] --typechecker test/slow/test_runner_typechecker_mode/basic.php

  # Use a specific typechecker binary
  # $argv[0] --hhserver-binary-path ~/code/hhvm/hphp/hack/bin/hh_server --typechecker .

EOT;
  return usage().$help;
}

function error($message) {
  print "$message\n";
  exit(1);
}

// If a user-supplied path is provided, let's make sure we have a valid
// executable.
function check_executable($path, $typechecker) {
  $type = $typechecker ? "HH_SERVER" : "HHVM";
  $rpath = realpath($path);
  $msg = "Provided ".$type." executable (".$path.") is not a file.\n"
       . "If using ".$type."_BIN, make sure that is set correctly.";
  if (!is_file($rpath)) {
    error($msg);
  }
  $output = array();
  exec($rpath . " --help 2> /dev/null", &$output);
  $str = implode($output);
  $msg = "Provided file (".$rpath.") is not a/an ".$type." executable.\n"
       . "If using ".$type."_BIN, make sure that is set correctly.";
  if (strpos($str, "Usage") !== 0) {
    error($msg);
  }
}

function hhvm_binary_routes() {
  return array(
    "buck"    => "/buck-out/gen/hphp/hhvm/hhvm",
    "cmake"   => "/hphp/hhvm"
  );
}

function hh_server_binary_routes() {
  return array(
    "buck"    => "/buck-out/gen/hphp/hack/src/hh_server",
    "cmake"   => "/hphp/hack/bin"
  );
}

function hh_codegen_binary_routes() {
  return array(
    "buck"    => "/buck-out/gen/hphp/hack/src/hh_single_compile",
    "cmake"   => "/hphp/hack/bin"
  );
}

// For Facebook: We have several build systems, and we can use any of them in
// the same code repo.  If multiple binaries exist, we want the onus to be on
// the user to specify a particular one because before we chose the buck one
// by default and that could cause unexpected results.
function check_for_multiple_default_binaries($typechecker) {
  // Env var we use in testing that'll pick which build system to use.
  if (getenv("FBCODE_BUILD_TOOL") !== false) {
    return;
  }

  $home = hphp_home();
  $routes = $typechecker ? hh_server_binary_routes() : hhvm_binary_routes();
  $binary = $typechecker ? "hh_server" : "hhvm";

  $found = array();
  foreach ($routes as $_ => $path) {
    $abs_path = $home . $path . "/" . $binary;
    if (file_exists($abs_path)) {
      $found[] = $abs_path;
    }
  }

  if (count($found) <= 1) {
    return;
  }

  $path_option = $typechecker ? "--hhserver-binary-path" : "--hhvm-binary-path";

  $msg = "Multiple binaries exist in this repo. \n";
  foreach ($found as $bin) {
    $msg .= " - " . $bin . "\n";
  }
  $msg .= "Are you in fbcode?  If so, remove a binary \n"
    . "or use the " . $path_option . " option to the test runner. \n"
    . "e.g., test/run ";
  if ($typechecker) {
    $msg .= "--typechecker";
  }
  $msg .= " " . $path_option . " /path/to/binary slow\n";
  error($msg);
}

function hphp_home() {
  if (is_testing_dso_extension()) {
    return realpath(__DIR__);
  }
  return realpath(__DIR__.'/../..');
}

function idx($array, $key, $default = null) {
  return isset($array[$key]) ? $array[$key] : $default;
}

function hhvm_path() {
  $file = "";
  if (getenv("HHVM_BIN") !== false) {
    $file = realpath(getenv("HHVM_BIN"));
  } else {
    $file = bin_root().'/hhvm';
  }

  if (!is_file($file)) {
    if (is_testing_dso_extension()) {
      exec("which hhvm 2> /dev/null", $output);
      if (isset($output[0]) && $output[0]) {
        return $output[0];
      }
      error("You need to specify hhvm bin with env HHVM_BIN");
    }

    error("$file doesn't exist. Did you forget to build first?");
  }
  return rel_path($file);
}

function hh_codegen_cmd($options) {
  $cmd = hh_codegen_path();
  $cmd .= ' -v Hack.Compiler.SourceMapping=1 ';
  if (isset($options['hackc'])) {
    $cmd .= ' --daemon';
  }

  return $cmd;
}

function bin_root() {
  if (getenv("HHVM_BIN") !== false) {
    return dirname(realpath(getenv("HHVM_BIN")));
  }

  $home = hphp_home();
  $env_tool = getenv("FBCODE_BUILD_TOOL");
  $routes = hhvm_binary_routes();

  if ($env_tool !== false) {
    return $home . $routes[$env_tool];
  }

  foreach ($routes as $_ => $path) {
    $dir = $home . $path;
    if (is_dir($dir)) {
      return $dir;
    }
  }

  return $home . $routes["cmake"];
}

function hh_server_path() {
  $file = "";
  if (getenv("HH_SERVER_BIN") !== false) {
    $file = realpath(getenv("HH_SERVER_BIN"));
  } else {
    $file = hh_server_bin_root().'/hh_server';
  }
  if (!is_file($file)) {
    error("$file doesn't exist. Did you forget to build first?");
  }
  return rel_path($file);
}

function hh_server_bin_root() {
  if (getenv("HH_SERVER_BIN") !== false) {
    return dirname(realpath(getenv("HH_SERVER_BIN")));
  }

  $home = hphp_home();
  $env_tool = getenv("FBCODE_BUILD_TOOL");
  $routes = hh_server_binary_routes();

  if ($env_tool !== false) {
    return $home . $routes[$env_tool];
  }

  foreach ($routes as $_ => $path) {
    $dir = $home . $path;
    if (is_dir($dir)) {
      return $dir;
    }
  }

  return $home . $routes["cmake"];
}

function hh_codegen_path() {
  $file = "";
  if (getenv("HH_CODEGEN_BIN") !== false) {
    $file = realpath(getenv("HH_CODEGEN_BIN"));
  } else {
    $file = hh_codegen_bin_root().'/hh_single_compile.opt';
  }
  if (!is_file($file)) {
    error("$file doesn't exist. Did you forget to build first?");
  }
  return rel_path($file);
}

function hh_codegen_bin_root() {
  $home = hphp_home();
  $env_tool = getenv("FBCODE_BUILD_TOOL");
  $routes = hh_codegen_binary_routes();

  if ($env_tool !== false) {
    return $home . $routes[$env_tool];
  }

  foreach ($routes as $_ => $path) {
    $dir = $home . $path;
    if (is_dir($dir)) {
      return $dir;
    }
  }

  return $home . $routes["cmake"];
}

function verify_hhbc() {
  if (getenv("VERIFY_HHBC") !== false) {
    return getenv($env_hhbc);
  }
  return bin_root().'/verify.hhbc';
}

function read_opts_file($file) {
  if (!file_exists($file)) {
    return "";
  }

  $fp = fopen($file, "r");

  $contents = "";
  while ($line = fgets($fp)) {
    // Compress out white space.
    $line = preg_replace('/\s+/', ' ', $line);

    // Discard simple line oriented ; and # comments to end of line
    // Comments at end of line (after payload) are not allowed.
    $line = preg_replace('/^ *;.*$/', ' ', $line);
    $line = preg_replace('/^ *#.*$/', ' ', $line);

    // Substitute in the directory name
    $line = str_replace('__DIR__', dirname($file), $line);

    $contents .= $line;
  }
  fclose($fp);
  return $contents;
}

// http://stackoverflow.com/questions/2637945/
function rel_path($to) {
  $from     = explode('/', getcwd().'/');
  $to       = explode('/', $to);
  $relPath  = $to;

  foreach ($from as $depth => $dir) {
    // find first non-matching dir.
    if ($dir === $to[$depth]) {
      // ignore this directory.
      array_shift(&$relPath);
    } else {
      // get number of remaining dirs to $from.
      $remaining = count($from) - $depth;
      if ($remaining > 1) {
        // add traversals up to first matching dir.
        $padLength = (count($relPath) + $remaining - 1) * -1;
        $relPath = array_pad($relPath, $padLength, '..');
        break;
      } else {
        $relPath[0] = './' . $relPath[0];
      }
    }
  }
  return implode('/', $relPath);
}

function get_options($argv) {
  # Options marked * affect test behavior, and need to be reported by list_tests
  $parameters = array(
    '*env:' => '',
    'exclude:' => 'e:',
    'exclude-pattern:' => 'E:',
    'exclude-recorded-failures:' => 'x:',
    'include:' => 'i:',
    'include-pattern:' => 'I:',
    '*repo' => 'r',
    '*repo-single' => '',
    '*repo-threads:' => '',
    '*hhbbc2' => '',
    '*mode:' => 'm:',
    '*server' => 's',
    '*cli-server' => 'S',
    'shuffle' => '',
    'help' => 'h',
    'verbose' => 'v',
    'testpilot' => '',
    'threads:' => '',
    '*args:' => 'a:',
    'log' => 'l',
    'failure-file:' => '',
    '*wholecfg' => '',
    '*hhas-round-trip' => '',
    'color' => 'c',
    'no-fun' => '',
    'cores' => '',
    'no-clean' => '',
    'list-tests' => '',
    '*relocate:' => '',
    '*recycle-tc:' => '',
    '*retranslate-all:' => '',
    '*jit-serialize:' => '',
    '*hhvm-binary-path:' => 'b:',
    '*typechecker' => '',
    '*vendor:' => '',
    '*hhserver-binary-path:' => '',
    'record-failures:' => '',
    '*hackc' => '',
    '*srcloc' => '',
    '*hack-only' => '',
    '*ignore-oids' => '',
    'jitsample:' => '',
    '*hh_single_type_check:' => '',
  );
  $options = array();
  $files = array();
  $recorded = array();

  /*
   * '-' argument causes all future arguments to be treated as filenames, even
   * if they would otherwise match a valid option. Otherwise, arguments starting
   * with '-' MUST match a valid option.
   */
  $force_file = false;

  for ($i = 1; $i < count($argv); $i++) {
    $arg = $argv[$i];

    if (strlen($arg) == 0) {
      continue;
    } else if ($force_file) {
      $files[] = $arg;
    } else if ($arg === '-') {
      $forcefile = true;
    } else if ($arg[0] === '-') {
      $found = false;

      foreach ($parameters as $long => $short) {
        if ($arg == '-'.str_replace(':', '', $short) ||
            $arg == '--'.str_replace(array(':', '*'), array('', ''), $long)) {
          $record = substr($long, 0, 1) === '*';
          if ($record) $recorded[] = $arg;
          if (substr($long, -1, 1) == ':') {
            $value = $argv[++$i];
            if ($record) $recorded[] = $value;
          } else {
            $value = true;
          }
          $options[str_replace(array(':', '*'), array('', ''), $long)] = $value;
          $found = true;
          break;
        }
      }

      if (!$found) {
        error(sprintf("Invalid argument: '%s'\nSee $argv[0] --help", $arg));
      }
    } else {
      $files[] = $arg;
    }
  }

  $GLOBALS['recorded_options'] = $recorded;

  if (isset($options['jit-serialize'])) {
    if (!isset($options['repo']) && !isset($options['repo-single'])) {
      echo "jit-serialize only works in repo mode\n";
      exit(1);
    }
    if (isset($options['mode']) && $options['mode'] != 'jit') {
      echo "jit-serialize only works in jit mode\n";
      exit(1);
    }
  }

  if (isset($options['repo']) && isset($options['hhas-round-trip'])) {
    echo "repo and hhas-round-trip are mutually exclusive options\n";
    exit(1);
  }

  $multi_request_modes = array_filter(multi_request_modes(),
                                      function($x) use ($options) {
                                        return isset($options[$x]);
                                      });
  if (count($multi_request_modes) > 1) {
    echo "The options\n -", implode("\n -", $multi_request_modes),
         "\nare mutually exclusive options\n";
    exit(1);
  }

  if (isset($options['hhbbc2']) || isset($options['repo-single'])) {
    if (isset($options['hhbbc2']) && isset($options['repo-single'])) {
      echo "repo-single and hhbbc2 are mutually exclusive options\n";
      exit(1);
    }
    $options['repo'] = true;
  }

  return array($options, $files);
}

/*
 * Return the path to $test relative to __DIR__, or false if __DIR__ does not
 * contain test.
 */
function canonical_path_from_base($test, $base) {
  $full = realpath($test);
  if (substr($full, 0, strlen($base)) === $base) {
    return substr($full, strlen($base) + 1);
  }
  $dirstat = stat($base);
  if (!is_array($dirstat)) return false;
  for ($p = dirname($full); $p && $p !== "/"; $p = dirname($p)) {
    $s = stat($p);
    if (!is_array($s)) continue;
    if ($s['ino'] === $dirstat['ino'] && $s['dev'] === $dirstat['dev']) {
      return substr($full, strlen($p) + 1);
    }
  }
  return false;
}

function canonical_path($test) {
  $attempt = canonical_path_from_base($test,__DIR__);
  if ($attempt === false) {
   return canonical_path_from_base($test, hphp_home());
  } else {
   return $attempt;
  }
}

/*
 * We support some 'special' file names, that just know where the test
 * suites are, to avoid typing 'hphp/test/foo'.
 */
function find_test_files($file) {
  $mappage = array(
    'quick'      => 'hphp/test/quick',
    'slow'       => 'hphp/test/slow',
    'spec'       => 'hphp/test/spec',
    'debugger'   => 'hphp/test/server/debugger/tests',
    'http'       => 'hphp/test/server/http/tests',
    'fastcgi'    => 'hphp/test/server/fastcgi/tests',
    'zend'       => 'hphp/test/zend/good',
    'facebook'   => 'hphp/facebook/test',

    // hhjs tests
    'hhjs_unit' => 'hphp/facebook/extensions/hhjs/test/ext_hhjs',
    'hhjs_functional' =>
      'hphp/facebook/extensions/hhjs/test/ext_hhjs_functional',

    // Subsets of zend tests.
    'zend_ext'    => 'hphp/test/zend/good/ext',
    'zend_ext_am' => 'hphp/test/zend/good/ext/[a-m]*',
    'zend_ext_nz' => 'hphp/test/zend/good/ext/[n-z]*',
    'zend_Zend'   => 'hphp/test/zend/good/Zend',
    'zend_tests'  => 'hphp/test/zend/good/tests',
  );

  if (isset($mappage[$file])) {
    $matches = glob(hphp_home().'/'.$mappage[$file]);
    if (count($matches) == 0) {
      error(sprintf(
        "Convenience test name '%s' is recognized but does not match any test ".
        "files (pattern = '%s', hphp_home = '%s')",
        $file, $mappage[$file], hphp_home()));
    }
    return $matches;
  } else {
    return array($file, );
  }
}

// Some tests have to be run together in the same test bucket, serially, one
// after other in order to avoid races and other collisions.
function serial_only_tests($tests) {
  if (is_testing_dso_extension()) {
    return array();
  }
  // Add a <testname>.php.serial file to make your test run in the serial
  // bucket.
  $serial_tests = array_filter(
    $tests,
    function($test) {
      return file_exists($test . '.serial');
    }
  );
  return $serial_tests;
}

function find_tests($files, array $options = null) {
  if (!$files) {
    $files = array('quick');
  }
  if ($files == array('all')) {
    $files = array('quick', 'slow', 'spec', 'zend', 'fastcgi');
    if (is_dir(hphp_home() . '/hphp/facebook/test')) {
      $files[] = 'facebook';
    }
  }
  $ft = array();
  foreach ($files as $file) {
    $ft = array_merge($ft, find_test_files($file));
  }
  $files = $ft;
  foreach ($files as $idx => $file) {
    if (!@stat($file)) {
      error("Not valid file or directory: '$file'");
    }
    $file = preg_replace(',//+,', '/', realpath($file));
    $file = preg_replace(',^'.getcwd().'/,', '', $file);
    $files[$idx] = $file;
  }
  $files = array_map('escapeshellarg', $files);
  $files = implode(' ', $files);
  if (isset($options['typechecker'])) {
    $tests = explode("\n", shell_exec(
      "find $files ".
      "-name '*.php' ".
      "-o -name '*.php.type-errors' ".
      "-o -name '*.hack' ".
      "-o -name '*.hack.type-errors'"
    ));
    // The above will get all the php files. Now filter out only the ones
    // that have a .hhconfig associated with it.
    $tests = array_filter(
      $tests,
      function($test) {
        return (file_exists($test . '.typechecker.expect') ||
                file_exists($test . '.typechecker.expectf')) &&
                file_exists($test . '.hhconfig');
      }
    );
  } else {
    $tests = explode("\n", shell_exec(
      "find $files '(' " .
          "-name '*.php' " .
          "-o -name '*.hack' " .
          "-o -name '*.js' " .
          "-o -name '*.php.type-errors' " .
          "-o -name '*.hack.type-errors' " .
          "-o -name '*.hhas' " .
        "')' " .
        "-not -regex '.*round_trip[.]hhas'"
    ));
  }
  if (!$tests) {
    error("Could not find any tests associated with your options.\n" .
          "Make sure your test path is correct and that you have " .
          "the right expect files for the tests you are trying to run.\n" .
          usage());
  }
  asort(&$tests);
  $tests = array_filter($tests);
  if (!empty($options['exclude'])) {
    $exclude = $options['exclude'];
    $tests = array_filter($tests, function($test) use ($exclude) {
      return (false === strpos($test, $exclude));
    });
  }
  if (!empty($options['exclude-pattern'])) {
    $exclude = $options['exclude-pattern'];
    $tests = array_filter($tests, function($test) use ($exclude) {
      return !preg_match($exclude, $test);
    });
  }
  if (!empty($options['exclude-recorded-failures'])) {
    $exclude_file = $options['exclude-recorded-failures'];
    $exclude = file($exclude_file, FILE_IGNORE_NEW_LINES);
    $tests = array_filter($tests, function($test) use ($exclude) {
      return (false === in_array(canonical_path($test), $exclude));
    });
  }
  if (!empty($options['include'])) {
    $include = $options['include'];
    $tests = array_filter($tests, function($test) use ($include) {
      return (false !== strpos($test, $include));
    });
  }
  if (!empty($options['include-pattern'])) {
    $include = $options['include-pattern'];
    $tests = array_filter($tests, function($test) use ($include) {
      return preg_match($include, $test);
    });
  }
  return $tests;
}

function list_tests($files, $options) {
  $args = implode(' ', $GLOBALS['recorded_options']);

  foreach (find_tests($files, $options) as $test) {
    print str_replace('\\', '\\\\',
                      Status::jsonEncode(
                        array('args' => $args, 'name' => $test)
                      )
                     )."\n";
  }
}

function find_test_ext($test, $ext, $configName='config') {
  if (is_file("{$test}.{$ext}")) {
    return "{$test}.{$ext}";
  }
  return find_file_for_dir(dirname($test), "{$configName}.{$ext}");
}

function find_file_for_dir($dir, $name) {
  // Handle the case where the $dir might come in as '.' because you
  // are running the test runner on a file from the same directory as
  // the test e.g., './mytest.php'. dirname() will give you the '.' when
  // you actually have a lot of path to traverse upwards like
  // /home/you/code/tests/mytest.php. Use realpath() to get that.
  $dir = realpath($dir);
  while ($dir !== '/' && is_dir($dir)) {
    $file = "$dir/$name";
    if (is_file($file)) {
      return $file;
    }
    $dir = dirname($dir);
  }
  $file = __DIR__.'/'.$name;
  if (file_exists($file)) {
    return $file;
  }
  return null;
}

function find_debug_config($test, $name) {
  $debug_config = find_file_for_dir(dirname($test), $name);
  if ($debug_config !== null) {
    return "-m debug --debug-config ".$debug_config;
  }
  return "";
}

function mode_cmd($options) {
  $repo_args = '';
  if (!isset($options['repo'])) {
    // Set the non-repo-mode shared repo.
    // When in repo mode, we set our own central path.
    $repo_args = "-vRepo.Local.Mode=-- -vRepo.Central.Path=".verify_hhbc();
  }
  $jit_args = "$repo_args -vEval.Jit=true";
  $mode = idx($options, 'mode', '');
  switch ($mode) {
    case '':
    case 'jit':
      return "$jit_args";
    case 'pgo':
      return $jit_args.
        ' -vEval.JitPGO=1'.
        ' -vEval.JitPGORegionSelector=hottrace'.
        ' -vEval.JitPGOHotOnly=0';
    case 'interp':
      return "$repo_args -vEval.Jit=0";
    case 'interp,jit':
      return array("$repo_args -vEval.Jit=0", $jit_args);
    default:
      error("-m must be one of jit | pgo | interp | interp,jit. Got: '$mode'");
  }
}

function extra_args($options): string {
  $args = $options['args'] ?? '';

  $vendor = $options['vendor'] ?? null;
  if ($vendor !== null) {
    $args .= ' -d auto_prepend_file=';
    $args .= escapeshellarg($vendor.'/hh_autoload.php');
  }
  return $args;
}

function hhvm_cmd_impl() {
  $args = func_get_args();
  $options = array_shift(&$args);
  $config = array_shift(&$args);
  $extra_args = $args;
  $modes = (array)mode_cmd($options);
  $cmds = array();
  foreach ($modes as $mode) {
    $args = array(
      hhvm_path(),
      '-c',
      $config,
      '-vEval.EnableArgsInBacktraces=true',
      '-vEval.EnableIntrinsicsExtension=true',
      '-vEval.HHIRInliningIgnoreHints=false',
      $mode,
      isset($options['wholecfg']) ? '-vEval.JitPGORegionSelector=wholecfg' : '',

      // load/store counters don't work on Ivy Bridge so disable for tests
      '-vEval.ProfileHWEnable=false',

      // use a fixed path for embedded data
      '-vEval.HackCompilerExtractPath='
        .escapeshellarg(bin_root().'/hackc_%{schema}'),
      '-vEval.EmbeddedDataExtractPath='
        .escapeshellarg(bin_root().'/hhvm_%{type}_%{buildid}'),

      extra_args($options),
    );

    if (isset($options['hackc'])) {
      $args[] = '-vEval.HackCompilerCommand="'.hh_codegen_cmd($options).'"';
      $args[] = '-vEval.HackCompilerUseEmbedded=false';
    }

    if (isset($options['relocate'])) {
      $args[] = '--count='.($options['relocate'] * 2);
      $args[] = '-vEval.JitAHotSize=6000000';
      $args[] = '-vEval.HotFuncCount=0';
      $args[] = '-vEval.PerfRelocate='.$options['relocate'];
    }

    if (isset($options['retranslate-all'])) {
      $args[] = '--count='.($options['retranslate-all'] * 2);
      $args[] = '-vEval.JitPGO=true';
      $args[] = '-vEval.JitRetranslateAllRequest='.$options['retranslate-all'];
      // Set to timeout.  We want requests to trigger retranslate all.
      $args[] = '-vEval.JitRetranslateAllSeconds=' . TIMEOUT_SECONDS;
    }

    if (isset($options['recycle-tc'])) {
      $args[] = '--count='.$options['recycle-tc'];
      $args[] = '-vEval.StressUnitCacheFreq=1';
      $args[] = '-vEval.EnableReusableTC=true';
    }

    if (isset($options['jit-serialize'])) {
      $args[] = '-vEval.JitPGO=true';
      $args[] = '-vEval.JitRetranslateAllRequest='.$options['jit-serialize'];
      // Set to timeout.  We want requests to trigger retranslate all.
      $args[] = '-vEval.JitRetranslateAllSeconds=' . TIMEOUT_SECONDS;
    }

    if (isset($options['hhas-round-trip'])) {
      $args[] = '-vEval.AllowHhas=1';
      $args[] = '-vEval.LoadFilepathFromUnitCache=1';
    }

    if (!isset($options['cores'])) {
      $args[] = '-vResourceLimit.CoreFileSize=0';
    }

    if (isset($options['hh_single_type_check'])) {
      $args[] = '--hh_single_type_check='.$options['hh_single_type_check'];
    }

    $cmds[] = implode(' ', array_merge($args, $extra_args));
  }
  if (count($cmds) != 1) return $cmds;
  return $cmds[0];
}

function repo_single($options, $test) {
  return isset($options['repo-single']) && !file_exists($test . ".hhbbc_opts");
}

// Return the command and the env to run it in.
function hhvm_cmd($options, $test, $test_run = null, $is_temp_file = false) {
  if ($test_run === null) {
    $test_run = $test;
  }
  // hdf support is only temporary until we fully migrate to ini
  // Discourage broad use.
  $hdf_suffix = ".use.for.ini.migration.testing.only.hdf";
  $hdf = file_exists($test.$hdf_suffix)
       ? '-c ' . $test . $hdf_suffix
       : "";
  $cmds = hhvm_cmd_impl(
    $options,
    find_test_ext($test, 'ini'),
    $hdf,
    find_debug_config($test, 'hphpd.ini'),
    read_opts_file(find_test_ext($test, 'opts')),
    '--file',
    escapeshellarg($test_run),
    $is_temp_file ? " --temp-file" : ""
  );

  $cmd = "";

  if (file_exists($test.'.verify')) {
    $cmd .= " -m verify";
  }

  if (isset($options['cli-server'])) {
    $config = find_file_for_dir(dirname($test), 'config.ini');
    $socket = $options['servers']['configs'][$config]['cli-socket'];
    $cmd .= ' -vEval.UseRemoteUnixServer=only';
    $cmd .= ' -vEval.UnixServerPath='.$socket;
    $cmd .= ' --count=3';
  }

  // Special support for tests that require a path to the current
  // test directory for things like prepend_file and append_file
  // testing.
  if (file_exists($test.'.ini')) {
    $contents = file_get_contents($test.'.ini');
    if (strpos($contents, '{PWD}') !== false) {
      $test_ini = tempnam('/tmp', $test).'.ini';
      file_put_contents($test_ini,
                        str_replace('{PWD}', dirname($test), $contents));
      $cmd .= " -c $test_ini";
    }
  }
  if ($hdf !== "") {
    $contents = file_get_contents($test.$hdf_suffix);
    if (strpos($contents, '{PWD}') !== false) {
      $test_hdf = tempnam('/tmp', $test).$hdf_suffix;
      file_put_contents($test_hdf,
                        str_replace('{PWD}', dirname($test), $contents));
      $cmd .= " -c $test_hdf";
    }
  }

  if (isset($options['repo'])) {
    $repo_suffix = repo_single($options, $test) ? 'hhbc' : 'hhbbc';

    $program = isset($options['hackc']) ? "hackc" : "hhvm";
    $hhbbc_repo = "\"$test.repo/$program.$repo_suffix\"";
    $cmd .= ' -vRepo.Authoritative=true -vRepo.Commit=0';
    $cmd .= " -vRepo.Central.Path=$hhbbc_repo";
  }

  if (isset($options['jitsample'])) {
    $cmd .= ' -vDeploymentId="' . $options['jitsample'] . '"';
    $cmd .= ' --instance-id="' . $test . '"';
    $cmd .= ' -vEval.JitSampleRate=1';
  }

  // Command line arguments
  $cli_args = find_test_ext($test, 'cli_args');
  if ($cli_args !== null) {
    $cmd .= " -- " . trim(file_get_contents($cli_args));
  }

  $env = $_ENV;
  $extra_env = array();

  // Apply the --env option
  if (isset($options['env'])) {
    $extra_env = array_merge($extra_env,
      explode(",", $options['env']));
  }

  // If there's an <test name>.env file then inject the contents of that into
  // the test environment.
  $env_file = find_test_ext($test, 'env');
  if ($env_file !== null) {
    $extra_env = array_merge($extra_env,
      explode("\n", trim(file_get_contents($env_file))));
  }

  if ($extra_env) {
    foreach ($extra_env as $arg) {
      $i = strpos($arg, '=');
      if ($i) {
        $key = substr($arg, 0, $i);
        $val = substr($arg, $i + 1);
        $env[$key] = $val;
      } else {
        unset($env[$arg]);
      }
    }
  }

  $in = find_test_ext($test, 'in');
  if ($in !== null) {
    $cmd .= ' < ' . escapeshellarg($in);
    // If we're piping the input into the command then setup a simple
    // dumb terminal so hhvm doesn't try to control it and pollute the
    // output with control characters, which could change depending on
    // a wide variety of terminal settings.
    $env["TERM"] = "dumb";
  }

  if (is_array($cmds)) {
    foreach ($cmds as $idx => $_) {
      $cmds[$idx] .= $cmd;
    }
    $cmd = $cmds;
  } else {
    $cmd = $cmds . $cmd;
  }

  return array($cmd, $env);
}

function hphp_cmd($options, $test, $program) {
  $extra_args = preg_replace("/-v\s*/", "-vRuntime.", extra_args($options));

  $compiler_args = "";
  if (isset($options['hackc'])) {
    $hh_single_compile = hh_codegen_path();
    $compiler_args = implode(" ", array(
      '-vRuntime.Eval.HackCompilerUseEmbedded=false',
      "-vRuntime.Eval.HackCompilerInheritConfig=true",
      "-vRuntime.Eval.HackCompilerCommand=\"${hh_single_compile} -v Hack.Compiler.SourceMapping=1 --daemon --dump-symbol-refs\""
    ));
  }

  return implode(" ", array(
    hhvm_path(),
    '--hphp',
    '-vUseHHBBC='. (repo_single($options, $test) ? 'true' : 'false'),
    '--config',
    find_test_ext($test, 'ini', 'hphp_config'),
    '-vRuntime.ResourceLimit.CoreFileSize=0',
    '-vRuntime.Eval.EnableIntrinsicsExtension=true',
    '-vRuntime.Eval.HackCompilerExtractPath='
      .escapeshellarg(bin_root().'/hackc_%{schema}'),
    '-vParserThreadCount=' . ($options['repo-threads'] ?? 1),
    '--nofork=1 -thhbc -l1 -k1',
    "-o \"$test.repo\" --program $program.hhbc \"$test\"",
    "-vRuntime.Repo.Local.Mode=rw -vRuntime.Repo.Local.Path=".verify_hhbc(),
    $extra_args,
    $compiler_args,
    read_opts_file("$test.hphp_opts"),
  ));
}

function hhbbc_cmd($options, $test, $program) {
  return implode(" ", array(
    hhvm_path(),
    '--hhbbc',
    '--no-logging',
    '--no-cores',
    '--parallel-num-threads=' . ($options['repo-threads'] ?? 1),
    '--hack-compiler-extract-path='
      .escapeshellarg(bin_root().'/hackc_%{schema}'),
    read_opts_file("$test.hhbbc_opts"),
    "-o \"$test.repo/$program.hhbbc\" \"$test.repo/$program.hhbc\"",
  ));
}

// Execute $cmd and return its output, including any stacktrace.log
// file it generated.
function exec_with_stack($cmd) {
  $proc = proc_open($cmd,
                    array(0 => array('pipe', 'r'),
                          1 => array('pipe', 'w'),
                          2 => array('pipe', 'w')), &$pipes);
  fclose($pipes[0]);
  $s = '';
  $all_selects_failed=true;
  $end = microtime(true) + TIMEOUT_SECONDS;
  $timedout = false;
  while (true) {
    $now = microtime(true);
    if ($now >= $end) break;
    $read = array($pipes[1], $pipes[2]);
    $write = null;
    $except = null;
    $available = @stream_select(&$read, &$write, &$except, (int)($end - $now));
    if ($available === false) {
      usleep(1000);
      $s .= "select failed:\n" . print_r(error_get_last(), true);
      continue;
    }
    $all_selects_failed=false;
    if ($available === 0) continue;
    # var_dump($read);
    foreach ($read as $pipe) {
      $t = fread($pipe, 4096);
      # var_dump($t);
      if ($t === false) continue;
      $s .= $t;
    }
    if (feof($pipes[1]) && feof($pipes[2])) break;
  }
  fclose($pipes[1]);
  fclose($pipes[2]);
  while (true) {
    $status = proc_get_status($proc);
    if (!$status['running']) break;
    $now = microtime(true);
    if ($now >= $end) {
      $timedout = true;
      exec('pkill -P ' . $status['pid'] . ' 2> /dev/null');
      posix_kill($status['pid'], SIGTERM);
    }
    usleep(1000);
  }
  proc_close($proc);
  if ($timedout) {
    if ($all_selects_failed) {
      return "All selects failed running `$cmd'\n\n$s";
    }
    return "Timed out running `$cmd'\n\n$s";
  }
  if (!$status['exitcode']) return true;
  $pid = $status['pid'];
  $stack =
    @file_get_contents("/tmp/stacktrace.$pid.log") ?:
    @file_get_contents("/var/tmp/cores/stacktrace.$pid.log");
  if ($stack !== false) {
    $s .= "\n" . $stack;
  }
  return "Running `$cmd' failed (".$status['exitcode']."):\n\n$s";
}

function repo_mode_compile($options, $test, $program) {
  $hphp = hphp_cmd($options, $test, $program);
  $result = exec_with_stack($hphp);
  if ($result === true && !repo_single($options, $test)) {
    $hhbbc = hhbbc_cmd($options, $test, $program);
    $result = exec_with_stack($hhbbc);
  }
  if ($result === true) return true;
  file_put_contents("$test.diff", $result);
}

function hh_server_cmd($options, $test) {
  // In order to run hh_server --check on only one file, we copy all of the
  // files associated with the test to a temporary directory, rename the
  // basename($test_file).hhconfig file to just .hhconfig and set the command
  // appropriately.
  $temp_dir = '/tmp/hh-test-runner-'.bin2hex(random_bytes(16));
  mkdir($temp_dir);
  foreach (glob($test . '*') as $test_file) {
    copy($test_file, $temp_dir . '/' . basename($test_file));
    if (strpos($test_file, '.hhconfig') !== false) {
      rename(
        $temp_dir . '/' . basename($test) . '.hhconfig',
        $temp_dir . '/.hhconfig'
      );
    } else if (strpos($test_file, '.type-errors') !== false) {
      // In order to actually run hh_server --check successfully, all files
      // named *.php.type-errors have to be renamed *.php
      rename(
        $temp_dir . '/' . basename($test_file),
        $temp_dir . '/' . str_replace('.type-errors', '', basename($test_file))
      );
    }
  }
  // Just copy all the .php.inc files, even if they are not related since
  // unrelated ones will be ignored anyway. This just makes it easier to
  // start with instead of doing a search inside the test file for requires
  // and includes and extracting it.
  foreach (glob(dirname($test) . "/*.inc.php") as $inc_file) {
    copy($inc_file, $temp_dir . '/' . basename($inc_file));
  }
  $cmd = hh_server_path() .  ' --check ' . $temp_dir;

  $vendor = $options['vendor'] ?? null;
  if ($vendor !== null) {
    $f = fopen($temp_dir.'/.hhconfig', 'a+');
    if (!is_resource($f)) {
      throw new Exception('failed to open hhconfig for append');
    }
    fprintf(
      $f,
      "\nextra_paths=%s\n",
      $vendor
    );
    fclose($f);
  }
  return array($cmd, ' ', $temp_dir);
}

class Status {
  private static $results = array();
  private static $mode = 0;

  private static $use_color = false;

  public static $nofork = false;
  private static $queue = null;
  private static $killed = false;
  public static $key;

  private static $overall_start_time = 0;
  private static $overall_end_time = 0;

  private static $tempdir = "";

  public static $passed = 0;
  public static $skipped = 0;
  public static $skip_reasons = array();
  public static $failed = 0;

  const MODE_NORMAL = 0;
  const MODE_VERBOSE = 1;
  const MODE_TESTPILOT = 3;
  const MODE_RECORD_FAILURES = 4;

  const MSG_STARTED = 7;
  const MSG_FINISHED = 1;
  const MSG_TEST_PASS = 2;
  const MSG_TEST_FAIL = 4;
  const MSG_TEST_SKIP = 5;
  const MSG_SERVER_RESTARTED = 6;

  const RED = 31;
  const GREEN = 32;
  const YELLOW = 33;
  const BLUE = 34;

  const PASS_SERVER = 0;
  const SKIP_SERVER = 1;
  const PASS_CLI = 2;

  private static function getTempDir() {
    self::$tempdir = sys_get_temp_dir();
    // Apparently some systems might not put the trailing slash
    if (substr(self::$tempdir, -1) !== "/") {
      self::$tempdir .= "/";
    }
    self::$tempdir .= getmypid().'-'.rand();
    mkdir(self::$tempdir);
  }

  public static function setMode($mode) {
    self::$mode = $mode;
  }

  public static function getMode() {
    return self::$mode;
  }

  public static function setUseColor($use) {
    self::$use_color = $use;
  }

  // Since we run the tests in forked processes, state is not shared
  // So we cannot keep a static variable adding individual test times.
  // But we can put the times files and add the values later.
  public static function setTestTime($time) {
    file_put_contents(tempnam(self::$tempdir, "trun"), $time);
  }

  // The total time running the tests if they were run serially.
  public static function addTestTimesSerial() {
    $time = 0;
    $files = scandir(self::$tempdir);
    foreach ($files as $file) {
      if (strpos($file, 'trun') === 0) {
        $time += floatval(file_get_contents(self::$tempdir . "/" . $file));
        unlink(self::$tempdir . "/" . $file);
      }
    }
    return $time;
  }

  public static function getOverallStartTime() {
    return self::$overall_start_time;
  }

  public static function getOverallEndTime() {
    return self::$overall_end_time;
  }

  public static function started() {
    self::getTempDir();
    self::send(self::MSG_STARTED, null);
    self::$overall_start_time = microtime(true);
  }

  public static function finished() {
    self::$overall_end_time = microtime(true);
    self::send(self::MSG_FINISHED, null);
  }

  public static function killQueue() {
    if (!self::$killed) {
      msg_remove_queue(self::$queue);
      self::$queue = null;
      self::$killed = true;
    }
  }

  public static function serverRestarted() {
    self::send(self::MSG_SERVER_RESTARTED, null);
  }

  public static function pass($test, $detail, $time, $stime, $etime) {
    array_push(&self::$results, array('name' => $test,
                                     'status' => 'passed',
                                     'start_time' => $stime,
                                     'end_time' => $etime,
                                     'time' => $time));
    $how = $detail === 'pass-server' ? self::PASS_SERVER :
      ($detail === 'skip-server' ? self::SKIP_SERVER : self::PASS_CLI);
    self::send(self::MSG_TEST_PASS, array($test, $how, $time, $stime, $etime));
  }

  public static function skip($test, $reason, $time, $stime, $etime) {
    if (self::getMode() === self::MODE_TESTPILOT) {
      /* testpilot needs a positive response for every test run, report
       * that this test isn't relevant so it can silently drop. */
      array_push(&self::$results, array('name' => $test,
                                       'status' => 'not_relevant',
                                       'start_time' => $stime,
                                       'end_time' => $etime,
                                       'time' => $time));
    } else {
      array_push(&self::$results, array('name' => $test,
                                       'status' => 'skipped',
                                       'start_time' => $stime,
                                       'end_time' => $etime,
                                       'time' => $time));
    }
    self::send(self::MSG_TEST_SKIP,
               array($test, $reason, $time, $stime, $etime));
  }

  public static function fail($test, $time, $stime, $etime, $diff) {
    array_push(&self::$results, array(
      'name' => $test,
      'status' => 'failed',
      'details' => self::utf8Sanitize($diff),
      'start_time' => $stime,
      'end_time' => $etime,
      'time' => $time
    ));
    self::send(self::MSG_TEST_FAIL, array($test, $time, $stime, $etime));
  }

  public static function handle_message($type, $message) {
    switch ($type) {
      case Status::MSG_STARTED:
        break;

      case Status::MSG_FINISHED:
        return false;

      case Status::MSG_SERVER_RESTARTED:
        switch (Status::getMode()) {
          case Status::MODE_NORMAL:
            if (!Status::hasCursorControl()) {
              Status::sayColor(Status::RED, 'x');
            }
            break;

          case Status::MODE_VERBOSE:
            Status::sayColor("$test ", Status::YELLOW, "failed",
                             " to talk to server\n");
            break;

          case Status::MODE_TESTPILOT:
            break;

          case Status::MODE_RECORD_FAILURES:
            break;
        }

      case Status::MSG_TEST_PASS:
        self::$passed++;
        list($test, $how, $time, $stime, $etime) = $message;
        switch (Status::getMode()) {
          case Status::MODE_NORMAL:
            if (!Status::hasCursorControl()) {
              if ($how == Status::SKIP_SERVER) {
                Status::sayColor(Status::RED, '.');
              } else {
                Status::sayColor(Status::GREEN,
                                 $how == Status::PASS_SERVER ? ',' : '.');
              }
            }
            break;

          case Status::MODE_VERBOSE:
            Status::sayColor("$test ", Status::GREEN,
                             sprintf("passed (%.2fs)\n", $time));
            break;

          case Status::MODE_TESTPILOT:
            Status::sayTestpilot($test, 'passed', $stime, $etime);
            break;

          case Status::MODE_RECORD_FAILURES:
            break;
        }
        break;

      case Status::MSG_TEST_SKIP:
        self::$skipped++;
        list($test, $reason, $time, $stime, $etime) = $message;
        self::$skip_reasons[$reason]++;

        switch (Status::getMode()) {
          case Status::MODE_NORMAL:
            if (!Status::hasCursorControl()) {
              Status::sayColor(Status::YELLOW, 's');
            }
            break;

          case Status::MODE_VERBOSE:
            Status::sayColor("$test ", Status::YELLOW, "skipped");

            if ($reason !== null) {
              Status::sayColor(" - $reason");
            }
            Status::sayColor(sprintf(" (%.2fs)\n", $time));
            break;

          case Status::MODE_TESTPILOT:
            Status::sayTestpilot($test, 'not_relevant', $stime, $etime);
            break;

          case Status::MODE_RECORD_FAILURES:
            break;
        }
        break;

      case Status::MSG_TEST_FAIL:
        self::$failed++;
        list($test, $time, $stime, $etime) = $message;
        switch (Status::getMode()) {
          case Status::MODE_NORMAL:
            if (Status::hasCursorControl()) {
              print "\033[2K\033[1G";
            }
            $diff = (string)@file_get_contents($test.'.diff');
            Status::sayColor(Status::RED, "\nFAILED",
                             ": $test\n$diff\n");
            break;

          case Status::MODE_VERBOSE:
            Status::sayColor("$test ", Status::RED,
                             sprintf("FAILED (%.2fs)\n", $time));
            break;

          case Status::MODE_TESTPILOT:
            Status::sayTestpilot($test, 'failed', $stime, $etime);
            break;

          case Status::MODE_RECORD_FAILURES:
            break;
        }
        break;

      default:
        error("Unknown message $type");
    }
    return true;
  }

  private static function send($type, $msg) {
    if (self::$killed) {
      return;
    }
    if (self::$nofork) {
      self::handle_message($type, $msg);
      return;
    }
    msg_send(self::getQueue(), $type, $msg);
  }

  /**
   * Takes a variable number of string arguments. If color output is enabled
   * and any one of the arguments is preceded by an integer (see the color
   * constants above), that argument will be given the indicated color.
   */
  public static function sayColor() {
    $args = func_get_args();
    while (count($args)) {
      $color = null;
      $str = array_shift(&$args);
      if (is_integer($str)) {
        $color = $str;
        if (self::$use_color) {
          print "\033[0;${color}m";
        }
        $str = array_shift(&$args);
      }

      print $str;

      if (self::$use_color && !is_null($color)) {
        print "\033[0m";
      }
    }
  }

  public static function sayTestpilot($test, $status, $stime, $etime) {
    $start = array('op' => 'start', 'test' => $test);
    $end = array('op' => 'test_done', 'test' => $test, 'status' => $status,
                 'start_time' => $stime, 'end_time' => $etime);
    if ($status == 'failed') {
      $end['details'] = self::utf8Sanitize(@file_get_contents("$test.diff"));
    }
    self::say($start, $end);
  }

  public static function getResults() {
    return self::$results;
  }

  /** Output is in the format expected by JsonTestRunner. */
  public static function say(/* ... */) {
    $data = array_map(function($row) {
      return self::jsonEncode($row) . "\n";
    }, func_get_args());
    fwrite(STDERR, implode("", $data));
  }

  public static function hasCursorControl() {
    // for runs on hudson-ci.org (aka jenkins).
    if (getenv("HUDSON_URL")) {
      return false;
    }
    // for runs on travis-ci.org
    if (getenv("TRAVIS")) {
      return false;
    }
    $stty = self::getSTTY();
    if (!$stty) {
      return false;
    }
    return strpos($stty, 'erase = <undef>') === false;
  }

  public static function getSTTY() {
    $descriptorspec = array(1 => array("pipe", "w"), 2 => array("pipe", "w"));
    $process = proc_open(
      'stty -a', $descriptorspec, &$pipes, null, null,
      array('suppress_errors' => true)
    );
    $stty = stream_get_contents($pipes[1]);
    proc_close($process);
    return $stty;
  }

  public static function utf8Sanitize($str) {
    if (!is_string($str)) {
      // We sometimes get called with the
      // return value of file_get_contents()
      // when fgc() has failed.
      return '';
    }

    return UConverter::transcode($str, 'UTF-8', 'UTF-8');
  }

  public static function jsonEncode($data) {
    // JSON_UNESCAPED_SLASHES is Zend 5.4+.
    if (defined("JSON_UNESCAPED_SLASHES")) {
      return json_encode($data, JSON_UNESCAPED_SLASHES);
    }

    $json = json_encode($data);
    return str_replace('\\/', '/', $json);
  }

  public static function getQueue() {
    if (!self::$queue) {
      self::$queue = msg_get_queue(self::$key);
    }
    return self::$queue;
  }
}

function clean_intermediate_files($test, $options) {
  if (isset($options['no-clean'])) {
    return;
  }
  $exts = array(
    // normal test output
    'out',
    'diff',
    // repo mode tests
    'repo',
    // tests in --hhas-round-trip mode
    'round_trip.hhas',
    // tests in --hhbbc2 mode
    'before.round_trip.hhas',
    'after.round_trip.hhas',
  );
  foreach ($exts as $ext) {
    $file = "$test.$ext";
    if (file_exists($file)) {
      if (is_dir($file)) {
        foreach(new RecursiveIteratorIterator(new
            RecursiveDirectoryIterator($file, FilesystemIterator::SKIP_DOTS),
            RecursiveIteratorIterator::CHILD_FIRST) as $path) {
          $path->isDir()
          ? rmdir($path->getPathname())
          : unlink($path->getPathname());
        }
        rmdir($file);
      } else {
        unlink($file);
      }
    }
  }
}

function run($options, $tests, $bad_test_file) {
  foreach ($tests as $test) {
    run_and_lock_test($options, $test);
  }
  file_put_contents($bad_test_file, json_encode(Status::getResults()));
  foreach (Status::getResults() as $result) {
    if ($result['status'] == 'failed') {
      return 1;
    }
  }
  return 0;
}

function is_hack_file($options, $test) {
  if (substr($test, -3) === '.hh') return true;

  $file = fopen($test, 'r');
  if ($file === false) return false;

  // Skip lines that are a shebang or whitespace.
  while (($line = fgets($file)) !== false) {
    $line = trim($line);
    if ($line === '' || substr($line, 0, 2) === '#!') continue;
    // Allow partial and strict, but don't count decl files as Hack code
    if ($line === '<?hh' || $line === '<?hh //strict') return true;
    break;
  }
  fclose($file);

  return false;
}

function skip_test($options, $test) {
  if (isset($options['hack-only']) &&
      substr($test, -5) !== '.hhas' &&
      !is_hack_file($options, $test)) {
    return 'skip-hack-only';
  }

  if (isset($options['cli-server']) && !can_run_server_test($test)) {
    return 'skip-server';
  }

  $skipif_test = find_test_ext($test, 'skipif');
  if (!$skipif_test) {
    return false;
  }

  // For now, run the .skipif in non-repo since building a repo for it is hard.
  $options_without_repo = $options;
  unset($options_without_repo['repo']);

  list($hhvm, $_) = hhvm_cmd($options_without_repo, $test, $skipif_test);
  if (is_array($hhvm)) $hhvm=$hhvm[0];

  $descriptorspec = array(
    0 => array("pipe", "r"),
    1 => array("pipe", "w"),
    2 => array("pipe", "w"),
  );
  $pipes = null;
  $process = proc_open("$hhvm $test 2>&1", $descriptorspec, &$pipes);
  if (!is_resource($process)) {
    // This is weird. We can't run HHVM but we probably shouldn't skip the test
    // since on a broken build everything will show up as skipped and give you a
    // SHIPIT.
    return false;
  }

  fclose($pipes[0]);
  $output = stream_get_contents($pipes[1]);
  fclose($pipes[1]);
  proc_close($process);

  // The standard php5 .skipif semantics is if the .skipif outputs ANYTHING
  // then it should be skipped. This is a poor design, but I'll just add a
  // small blacklist of things that are really bad if they are output so we
  // surface the errors in the tests themselves.
  if (stripos($output, 'segmentation fault') !== false) {
    return false;
  }

  return strlen($output) === 0 ? false : 'skip-skipif';
}

function comp_line($l1, $l2, $is_reg) {
  if ($is_reg) {
    return preg_match('/^'. $l1 . '$/s', $l2);
  } else {
    return !strcmp($l1, $l2);
  }
}

function count_array_diff($ar1, $ar2, $is_reg, $w, $idx1, $idx2, $cnt1, $cnt2,
                          $steps) {
  $equal = 0;

  while ($idx1 < $cnt1 && $idx2 < $cnt2 && comp_line($ar1[$idx1], $ar2[$idx2],
                                                     $is_reg)) {
    $idx1++;
    $idx2++;
    $equal++;
    $steps--;
  }
  if (--$steps > 0) {
    $eq1 = 0;
    $st = $steps / 2;

    for ($ofs1 = $idx1 + 1; $ofs1 < $cnt1 && $st-- > 0; $ofs1++) {
      $eq = @count_array_diff($ar1, $ar2, $is_reg, $w, $ofs1, $idx2, $cnt1,
                              $cnt2, $st);

      if ($eq > $eq1) {
        $eq1 = $eq;
      }
    }

    $eq2 = 0;
    $st = $steps;

    for ($ofs2 = $idx2 + 1; $ofs2 < $cnt2 && $st-- > 0; $ofs2++) {
      $eq = @count_array_diff($ar1, $ar2, $is_reg, $w, $idx1, $ofs2, $cnt1, $cnt2, $st);
      if ($eq > $eq2) {
        $eq2 = $eq;
      }
    }

    if ($eq1 > $eq2) {
      $equal += $eq1;
    } else if ($eq2 > 0) {
      $equal += $eq2;
    }
  }

  return $equal;
}

function generate_array_diff($ar1, $ar2, $is_reg, $w) {
  $idx1 = 0; $ofs1 = 0; $cnt1 = @count($ar1);
  $idx2 = 0; $ofs2 = 0; $cnt2 = @count($ar2);
  $diff = array();
  $old1 = array();
  $old2 = array();

  while ($idx1 < $cnt1 && $idx2 < $cnt2) {

    if (comp_line($ar1[$idx1], $ar2[$idx2], $is_reg)) {
      $idx1++;
      $idx2++;
      continue;
    } else {

      $c1 = @count_array_diff($ar1, $ar2, $is_reg, $w, $idx1+1, $idx2, $cnt1,
                              $cnt2, 10);
      $c2 = @count_array_diff($ar1, $ar2, $is_reg, $w, $idx1, $idx2+1, $cnt1,
                              $cnt2, 10);

      if ($c1 > $c2) {
        $old1[$idx1] = sprintf("%03d- ", $idx1+1) . $w[$idx1++];
        $last = 1;
      } else if ($c2 > 0) {
        $old2[$idx2] = sprintf("%03d+ ", $idx2+1) . $ar2[$idx2++];
        $last = 2;
      } else {
        $old1[$idx1] = sprintf("%03d- ", $idx1+1) . $w[$idx1++];
        $old2[$idx2] = sprintf("%03d+ ", $idx2+1) . $ar2[$idx2++];
      }
    }
  }

  reset(&$old1); $k1 = key(&$old1); $l1 = -2;
  reset(&$old2); $k2 = key(&$old2); $l2 = -2;

  while ($k1 !== null || $k2 !== null) {

    if ($k1 == $l1 + 1 || $k2 === null) {
      $l1 = $k1;
      $diff[] = current(&$old1);
      $k1 = next(&$old1) ? key(&$old1) : null;
    } else if ($k2 == $l2 + 1 || $k1 === null) {
      $l2 = $k2;
      $diff[] = current(&$old2);
      $k2 = next(&$old2) ? key(&$old2) : null;
    } else if ($k1 < $k2) {
      $l1 = $k1;
      $diff[] = current(&$old1);
      $k1 = next(&$old1) ? key(&$old1) : null;
    } else {
      $l2 = $k2;
      $diff[] = current(&$old2);
      $k2 = next(&$old2) ? key(&$old2) : null;
    }
  }

  while ($idx1 < $cnt1) {
    $diff[] = sprintf("%03d- ", $idx1 + 1) . $w[$idx1++];
  }

  while ($idx2 < $cnt2) {
    $diff[] = sprintf("%03d+ ", $idx2 + 1) . $ar2[$idx2++];
  }

  return $diff;
}

function generate_diff($wanted, $wanted_re, $output)
{
  $w = explode("\n", $wanted);
  $o = explode("\n", $output);
  if (is_null($wanted_re)) {
    $r = $w;
  } else {
    if (preg_match('/^\((.*)\)\{(\d+)\}$/s', $wanted_re, &$m)) {
      $t = explode("\n", $m[1]);
      $r = array();
      $w2 = array();
      for ($i = 0; $i < $m[2]; $i++) {
        foreach ($t as $v) {
          $r[] = $v;
        }
        foreach ($w as $v) {
          $w2[] = $v;
        }
      }
      $w = $wanted === $wanted_re ? $r : $w2;
    } else {
      $r = explode("\n", $wanted_re);
    }
  }
  $diff = generate_array_diff($r, $o, !is_null($wanted_re), $w);

  return implode("\r\n", $diff);
}

function dump_hhas_cmd($hhvm_cmd, $test, $hhas_file) {
  $dump_flags = implode(' ', array(
    '-vEval.AllowHhas=true',
    '-vEval.DumpHhas=1',
    '-vEval.DumpHhasToFile='.escapeshellarg($hhas_file),
    '-vEval.LoadFilepathFromUnitCache=0',
  ));
  $cmd = str_replace(' -- ', " $dump_flags -- ", $hhvm_cmd);
  if ($cmd == $hhvm_cmd) $cmd .= " $dump_flags";
  return $cmd;
}

function dump_hhas_to_temp($hhvm_cmd, $test) {
  $temp_file = $test . '.round_trip.hhas';
  $cmd = dump_hhas_cmd($hhvm_cmd, $test, $temp_file);
  system("$cmd &> /dev/null", &$ret);
  return $ret === 0 ? $temp_file : false;
}

const HHAS_EXT = '.hhas';
function can_run_server_test($test) {
  return
    !is_file("$test.noserver") &&
    !find_test_ext($test, 'opts') &&
    !is_file("$test.ini") &&
    !is_file("$test.onlyrepo") &&
    !is_file("$test.use.for.ini.migration.testing.only.hdf") &&
    strpos($test, 'quick/debugger') === false &&
    strpos($test, 'quick/xenon') === false &&
    strpos($test, 'slow/streams/') === false &&
    strpos($test, 'slow/ext_mongo/') === false &&
    strpos($test, 'slow/ext_oauth/') === false &&
    strpos($test, 'slow/ext_vsdebug/') === false &&
    strpos($test, 'slow/ext_yaml/') === false &&
    strpos($test, 'slow/ext_xdebug/') === false &&
    strpos($test, 'slow/debugger/') === false &&
    strpos($test, 'slow/type_profiler/debugger/') === false &&
    strpos($test, 'zend/good/ext/standard/tests/array/') === false &&
    strpos($test, 'zend/good/ext/ftp') === false &&
    strrpos($test, HHAS_EXT) !== (strlen($test) - strlen(HHAS_EXT))
    ;
}

const SERVER_TIMEOUT = 45;
function run_config_server($options, $test) {
  if (!isset($options['server']) || !can_run_server_test($test)) {
    return null;
  }

  $config = find_file_for_dir(dirname($test), 'config.ini');
  $port = $options['servers']['configs'][$config]['port'];
  $ch = curl_init("localhost:$port/$test");
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  curl_setopt($ch, CURLOPT_TIMEOUT, SERVER_TIMEOUT);
  curl_setopt($ch, CURLOPT_BINARYTRANSFER, true);
  $output = curl_exec($ch);
  if ($output === false) {
    // The server probably crashed so fall back to cli to determine if this was
    // the test that caused the crash. Our parent process will see that the
    // server died and restart it.
    if (getenv('HHVM_TEST_SERVER_LOG')) {
      printf("Curl failed: %d\n", curl_errno($ch));
    }
    return null;
  }
  curl_close($ch);
  $output = trim($output);

  return array($output, '');
}

function run_config_cli($options, $test, $cmd, $cmd_env) {
  if (isset($options['log']) && !isset($options['typechecker'])) {
    $cmd_env['TRACE'] = 'printir:1';
    $cmd_env['HPHP_TRACE_FILE'] = $test . '.log';
  }

  $descriptorspec = array(
    0 => array("pipe", "r"),
    1 => array("pipe", "w"),
    2 => array("pipe", "w"),
  );
  $pipes = null;
  if (isset($options['typechecker'])) {
    $process = proc_open(
      "$cmd 2>/dev/null", $descriptorspec, &$pipes, null, $cmd_env
    );
  } else {
    $process = proc_open("$cmd 2>&1", $descriptorspec, &$pipes, null, $cmd_env);
  }
  if (!is_resource($process)) {
    file_put_contents("$test.diff", "Couldn't invoke $cmd");
    return false;
  }

  fclose($pipes[0]);
  $output = stream_get_contents($pipes[1]);
  $output = trim($output);
  $stderr = stream_get_contents($pipes[2]);
  fclose($pipes[1]);
  fclose($pipes[2]);
  proc_close($process);

  return array($output, $stderr);
}

function replace_object_resource_ids($str, $replacement) {
  $str = preg_replace(
    '/(object\([^)]+\)#)\d+/', '\1'.$replacement, $str
  );
  return preg_replace(
    '/resource\(\d+\)/', "resource($replacement)", $str
  );
}

function run_config_post($outputs, $test, $options) {
  $output = $outputs[0];
  $stderr = $outputs[1];
  file_put_contents("$test.out", $output);

  // hhvm redirects errors to stdout, so anything on stderr is really bad.
  if ($stderr) {
    file_put_contents(
      "$test.diff",
      "Test failed because the process wrote on stderr:\n$stderr"
    );
    return false;
  }

  // Needed for testing non-hhvm binaries that don't actually run the code
  // e.g. parser/test/parse_tester.cpp.
  if ($output == "FORCE PASS") {
    return true;
  }

  $repeats = 0;

  if (isset($options['relocate'])) {
    $repeats = $options['relocate'] * 2;
  }

  if (isset($options['retranslate-all'])) {
    $repeats = $options['retranslate-all'] * 2;
  }

  if (isset($options['recycle-tc'])) {
    $repeats = $options['recycle-tc'];
  }

  if (isset($options['cli-server'])) {
    $repeats = 3;
  }

  list($file, $type) = get_expect_file_and_type($test, $options);
  if ($file === null || $type === null) {
    file_put_contents(
      "$test.diff", "No $test.expect, $test.expectf, " .
      "$test.hhvm.expect, $test.hhvm.expectf, " .
      "$test.typechecker.expect, $test.typechecker.expectf, " .
      "nor $test.expectregex. If $test is meant to be included by other ".
      "tests, use a different file extension.\n"
    );
    return false;
  }

  $is_tc = isset($options['typechecker']);
  if ((!$is_tc && ($type === 'expect' || $type === 'hhvm.expect')) ||
      ($is_tc && $type === 'typechecker.expect')) {
    $wanted = trim(file_get_contents($file));
    if (isset($options['ignore-oids'])) {
      $output = replace_object_resource_ids($output, 'n');
      $wanted = replace_object_resource_ids($wanted, 'n');
    }

    if (!$repeats) {
      $passed = !strcmp($output, $wanted);
      if (!$passed) {
        file_put_contents("$test.diff", generate_diff($wanted, null, $output));
      }
      return $passed;
    }
    $wanted_re = preg_quote($wanted, '/');
  } else if ((!$is_tc && ($type === 'expectf' || $type === 'hhvm.expectf')) ||
             ($is_tc && $type === 'typechecker.expectf')) {
    $wanted = trim(file_get_contents($file));
    if (isset($options['ignore-oids'])) {
      $wanted = replace_object_resource_ids($wanted, '%d');
    }
    $wanted_re = $wanted;

    // do preg_quote, but miss out any %r delimited sections.
    $temp = "";
    $r = "%r";
    $startOffset = 0;
    $length = strlen($wanted_re);
    while ($startOffset < $length) {
      $start = strpos($wanted_re, $r, $startOffset);
      if ($start !== false) {
        // we have found a start tag.
        $end = strpos($wanted_re, $r, $start+2);
        if ($end === false) {
          // unbalanced tag, ignore it.
          $end = $start = $length;
        }
      } else {
        // no more %r sections.
        $start = $end = $length;
      }
      // quote a non re portion of the string.
      $temp = $temp.preg_quote(substr($wanted_re, $startOffset,
                                      ($start - $startOffset)),  '/');
      // add the re unquoted.
      if ($end > $start) {
        $temp = $temp.'('.substr($wanted_re, $start+2, ($end - $start-2)).')';
      }
      $startOffset = $end + 2;
    }
    $wanted_re = $temp;

    $wanted_re = str_replace(
      array('%binary_string_optional%'),
      'string',
      $wanted_re
    );
    $wanted_re = str_replace(
      array('%unicode_string_optional%'),
      'string',
      $wanted_re
    );
    $wanted_re = str_replace(
      array('%unicode\|string%', '%string\|unicode%'),
      'string',
      $wanted_re
    );
    $wanted_re = str_replace(
      array('%u\|b%', '%b\|u%'),
      '',
      $wanted_re
    );
    // Stick to basics.
    $wanted_re = str_replace('%e', '\\' . DIRECTORY_SEPARATOR, $wanted_re);
    $wanted_re = str_replace('%s', '[^\r\n]+', $wanted_re);
    $wanted_re = str_replace('%S', '[^\r\n]*', $wanted_re);
    $wanted_re = str_replace('%a', '.+', $wanted_re);
    $wanted_re = str_replace('%A', '.*', $wanted_re);
    $wanted_re = str_replace('%w', '\s*', $wanted_re);
    $wanted_re = str_replace('%i', '[+-]?\d+', $wanted_re);
    $wanted_re = str_replace('%d', '\d+', $wanted_re);
    $wanted_re = str_replace('%x', '[0-9a-fA-F]+', $wanted_re);
    // %f allows two points "-.0.0" but that is the best *simple* expression.
    $wanted_re = str_replace('%f', '[+-]?\.?\d+\.?\d*(?:[Ee][+-]?\d+)?',
                             $wanted_re);
    $wanted_re = str_replace('%c', '.', $wanted_re);
    // must be last.
    $wanted_re = str_replace('%%', '%%?', $wanted_re);

    // Normalize newlines.
    $wanted_re = preg_replace("/(\r\n?|\n)/", "\n", $wanted_re);
    $output    = preg_replace("/(\r\n?|\n)/", "\n", $output);
  } else if (!$is_tc && $type === 'expectregex') {
    $wanted_re = trim(file_get_contents($file));
  } else {
    throw new Exception("Unsupported expect file type: ".$type);
  }

  if ($repeats) {
    $wanted_re = "($wanted_re\s*)".'{'.$repeats.'}';
  }
  if (!isset($wanted)) $wanted = $wanted_re;
  $passed = @preg_match("/^$wanted_re\$/s", $output);
  if ($passed === false && $repeats) {
    // $repeats can cause the regex to become too big, and fail
    // to compile.
    return 'skip-repeats-fail';
  }
  if (!$passed) {
    $diff = generate_diff($wanted_re, $wanted_re, $output);
    if ($passed === false && $diff === "") {
      // the preg match failed, probably because the regex was too complex,
      // but since the line by line diff came up empty, we're fine
      $passed = 1;
    } else {
      file_put_contents("$test.diff", $diff);
    }
  }
  return $passed;
}

function timeout_prefix() {
  if (is_executable('/usr/bin/timeout')) {
    return '/usr/bin/timeout ' . TIMEOUT_SECONDS . ' ';
  } else {
    return __DIR__.'/../tools/timeout.sh -t ' . TIMEOUT_SECONDS . ' ';
  }
}

function run_one_config($options, $test, $cmd, $cmd_env) {
  if (is_array($cmd)) {
    $result = 'skip-empty-cmd';
    foreach ($cmd as $c) {
      $result = run_one_config($options, $test, $c, $cmd_env);
      if (!$result) return $result;
    }
    return $result;
  }
  $cmd = timeout_prefix() . $cmd;
  $outputs = run_config_cli($options, $test, $cmd, $cmd_env);
  if ($outputs === false) return false;
  return run_config_post($outputs, $test, $options);
}

function run_and_lock_test($options, $test) {
  $stime = time();
  $time = microtime(true);
  $failmsg = "";
  $status = false;
  $lock = fopen($test, 'r');
  if (!$lock || !flock($lock, LOCK_EX)) {
    $failmsg = "Failed to lock test";
    if ($lock) fclose($lock);
    $lock = null;
  } else {
    if (isset($options['typechecker'])) {
      $status = run_typechecker_test($options, $test);
    } else {
      $status = run_test($options, $test);
    }
  }
  $time = microtime(true) - $time;
  $etime = time();
  Status::setTestTime($time);
  if ($lock) {
    if ($status) {
      clean_intermediate_files($test, $options);
    } else if ($failmsg === '') {
      $failmsg = @file_get_contents("$test.diff");
      if (!$failmsg) $failmsg = "Test failed with empty diff";
    }
    if (!flock($lock, LOCK_UN)) {
      if ($failmsg !== '') $failmsg .= "\n";
      $failmsg .= "Failed to release test lock";
    }
    if (!fclose($lock)) {
      if ($failmsg !== '') $failmsg .= "\n";
      $failmsg .= "Failed to close lock file";
    }
  }
  if ($failmsg !== "") {
    Status::fail($test, $time, $stime, $etime, $failmsg);
  } else if (is_string($status) && substr($status, 0, 4) === 'skip') {
    if (strlen($status) > 5 && substr($status, 0, 5) === 'skip-') {
      Status::skip($test, substr($status, 5), $time, $stime, $etime);
    } else {
      Status::fail($test, $time, $stime, $etime, "invalid skip status $status");
    }
  } else if ($status) {
    Status::pass($test, $status, $time, $stime, $etime);
  } else {
    Status::fail($test, $time, $stime, $etime, "Unknown failure");
  }
}

function run_typechecker_test($options, $test) {
  $skip_reason = skip_test($options, $test);
  if ($skip_reason !== false) return $skip_reason;
  if (!file_exists($test . ".hhconfig")) return 'skip-no-hhconfig';
  list($hh_server, $hh_server_env, $temp_dir) = hh_server_cmd($options, $test);
  $result =  run_one_config($options, $test, $hh_server, $hh_server_env);
  // Remove the temporary directory.
  if (!isset($options['no-clean'])) {
    shell_exec('rm -rf ' . $temp_dir);
  }
  return $result;
}

function run_test($options, $test) {
  $skip_reason = skip_test($options, $test);
  if ($skip_reason !== false) return $skip_reason;

  // Skip tests that don't make sense in modes where we dump/compare hhas
  $no_hhas_tag = '.nodumphhas';
  $no_hhas = file_exists($test.$no_hhas_tag) ||
             file_exists(dirname($test).'/'.$no_hhas_tag);
  if ($no_hhas && (
    isset($options['hhbbc2']) ||
    isset($options['hhas-round-trip'])
  )) {
    return 'skip-nodumphhas';
  }

  list($hhvm, $hhvm_env) = hhvm_cmd($options, $test);
  if (has_multi_request_mode($options)) {
    if (isset($options['jit-serialize']) || isset($options['cli-server'])) {
      if (preg_grep('/ --count[ =][0-9]+ /', (array)$hhvm)) {
        return 'skip-count';
      }
    } else {
      if (preg_grep('/ --count[ =][0-9]+ .* --count[ =][0-9]+ /',
                    (array)$hhvm)) {
        return 'skip-count';
      }
    }
  }

  if (file_exists($test . ".verify") && (has_multi_request_mode($options) ||
                                         isset($options['repo']))) {
      return 'skip-verify';
  }

  if (isset($options['repo'])) {
    if (preg_grep('/-m debug/', (array)$hhvm) || file_exists($test.'.norepo')) {
      return 'skip-norepo';
    }

    $hphp_hhvm_repo = "$test.repo/hhvm.hhbc";
    $hhbbc_hhvm_repo = "$test.repo/hhvm.hhbbc";
    $hphp_hackc_repo = "$test.repo/hackc.hhbc";
    $hhbbc_hackc_repo = "$test.repo/hackc.hhbbc";
    shell_exec("rm -f \"$hphp_hhvm_repo\" \"$hhbbc_hhvm_repo\" \"$hphp_hackc_repo\" \"$hhbbc_hackc_repo\" ");

    $program = isset($options['hackc']) ? "hackc" : "hhvm";
    if (!repo_mode_compile($options, $test, $program)) {
      return false;
    }

    if (isset($options['hhbbc2'])) {
      $hhas_temp1 = dump_hhas_to_temp($hhvm, "$test.before");
      if ($hhas_temp1 === false) {
        file_put_contents(
          "$test.diff",
          "dumping hhas after first hhbbc pass failed"
        );
        return false;
      }
      shell_exec("mv $test.repo/$program.hhbbc $test.repo/$program.hhbc");
      $hhbbc = hhbbc_cmd($options, $test, $program);
      $result = exec_with_stack($hhbbc);
      if ($result !== true) {
        file_put_contents("$test.diff", $result);
        return false;
      }
      $hhas_temp2 = dump_hhas_to_temp($hhvm, "$test.after");
      if ($hhas_temp2 === false) {
        file_put_contents(
          "$test.diff",
          "dumping hhas after second hhbbc pass failed"
        );
        return false;
      }
      $diff = shell_exec("diff $hhas_temp1 $hhas_temp2 | wc -l");
      if (trim($diff) != '0') {
        shell_exec("diff $hhas_temp1 $hhas_temp2 > $test.diff");
        return false;
      }
    }

    if (isset($options['jit-serialize'])) {
      $cmd = timeout_prefix() .
        jit_serialize_option($hhvm, $test, $options, true);
      $outputs = run_config_cli($options, $test, $cmd, $hhvm_env);
      if ($outputs === false) return false;
      $hhvm = jit_serialize_option($hhvm, $test, $options, false);
    }

    return run_one_config($options, $test, $hhvm, $hhvm_env);
  }

  if (file_exists($test.'.onlyrepo')) {
    return 'skip-onlyrepo';
  }

  if (isset($options['hhas-round-trip'])) {
    if (substr($test, -5) === ".hhas") return 'skip-hhas';
    $hhas_temp = dump_hhas_to_temp($hhvm, $test);
    if ($hhas_temp === false) {
      $err = "system failed: " .
        dump_hhas_cmd($hhvm, $test, $test.'.round_trip.hhas') .
        "\n";
      file_put_contents("$test.diff", $err);
      return false;
    }
    list($hhvm, $hhvm_env) = hhvm_cmd($options, $test, $hhas_temp);
  }

  if ($outputs = run_config_server($options, $test)) {
    return run_config_post($outputs, $test, $options) ? 'pass-server'
      : (run_one_config($options, $test, $hhvm, $hhvm_env) ? 'skip-server'
         : false);
  }
  return run_one_config($options, $test, $hhvm, $hhvm_env);
}

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
  return 2; // default when we don't know how to detect.
}

function make_header($str) {
  return "\n\033[0;33m".$str."\033[0m\n";
}

function print_commands($tests, $options) {
  print make_header("Run these by hand:");

  foreach ($tests as $test) {
    if (isset($options['typechecker'])) {
      list($command, $_, ) = hh_server_cmd($options, $test);
    } else {
      list($command, $_) = hhvm_cmd($options, $test);
    }
    if (!isset($options['repo'])) {
      foreach ((array)$command as $c) {
        print "$c\n";
      }
      continue;
    }

    // How to run it with hhbbc:
    $program = isset($options['hackc']) ? "hackc" : "hhvm";
    $hhbbc_cmds = hphp_cmd($options, $test, $program)."\n";
    if (!repo_single($options, $test)) {
      $hhbbc_cmd  = hhbbc_cmd($options, $test, $program)."\n";
      $hhbbc_cmds .= $hhbbc_cmd;
      if (isset($options['hhbbc2'])) {
        foreach ((array)$command as $c) {
          $hhbbc_cmds .=
            $c." -vEval.DumpHhas=1 > $test.before.round_trip.hhas\n";
        }
        $hhbbc_cmds .=
          "mv $test.repo/$program.hhbbc $test.repo/$program.hhbc\n";
        $hhbbc_cmds .= $hhbbc_cmd;
        foreach ((array)$command as $c) {
          $hhbbc_cmds .=
            $c." -vEval.DumpHhas=1 > $test.after.round_trip.hhas\n";
        }
        $hhbbc_cmds .=
          "diff $test.before.round_trip.hhas $test.after.round_trip.hhas\n";
      }
    }
    if (isset($options['jit-serialize'])) {
      $hhbbc_cmds .=
        jit_serialize_option($command, $test, $options, true) . "\n";
      $command = jit_serialize_option($command, $test, $options, false);
    }
    foreach ((array)$command as $c) {
      $hhbbc_cmds .= $c."\n";
    }
    print "$hhbbc_cmds\n";
  }
}

function msg_loop($num_tests, $queue) {
  $do_progress =
    (
      Status::getMode() === Status::MODE_NORMAL ||
      Status::getMode() === Status::MODE_RECORD_FAILURES
    ) &&
    Status::hasCursorControl();

  if ($do_progress) {
    $stty = strtolower(Status::getSTTY());
    preg_match_all("/columns ([0-9]+);/", $stty, &$output);
    if (!isset($output[1][0])) {
      // because BSD has to be different
      preg_match_all("/([0-9]+) columns;/", $stty, &$output);
    }
    if (!isset($output[1][0])) {
      $do_progress = false;
    } else {
      $cols = $output[1][0];
    }
  }

  while (true) {
    if (!msg_receive($queue, 0, &$type, 1024, &$message)) {
      error("msg_receive failed");
    }

    if (!Status::handle_message($type, $message)) break;

    if ($do_progress) {
      $total_run = (Status::$skipped + Status::$failed + Status::$passed);
      $bar_cols = ($cols - 45);

      $passed_ticks  = round($bar_cols * (Status::$passed  / $num_tests));
      $skipped_ticks = round($bar_cols * (Status::$skipped / $num_tests));
      $failed_ticks  = round($bar_cols * (Status::$failed  / $num_tests));

      $fill = $bar_cols - ($passed_ticks + $skipped_ticks + $failed_ticks);
      if ($fill < 0) $fill = 0;

      $fill = str_repeat('-', $fill);

      $passed_ticks = str_repeat('#',  $passed_ticks);
      $skipped_ticks = str_repeat('#', $skipped_ticks);
      $failed_ticks = str_repeat('#',  $failed_ticks);

      echo
        "\033[2K\033[1G[",
        "\033[0;32m$passed_ticks",
        "\033[33m$skipped_ticks",
        "\033[31m$failed_ticks",
        "\033[0m$fill] ($total_run/$num_tests) ",
        "(", Status::$skipped, " skipped,", Status::$failed, " failed)";
    }
  }

  if ($do_progress) {
    print "\033[2K\033[1G";
    if (Status::$skipped > 0) {
      print Status::$skipped ." tests \033[1;33mskipped\033[0m\n";
      arsort(&Status::$skip_reasons);
      foreach (Status::$skip_reasons as $reason => $count) {
        printf("%12s: %d\n", $reason, $count);
      }
    }
  }
}

function print_success($tests, $results, $options) {
  // We didn't run any tests, not even skipped. Clowntown!
  if (!$tests) {
    print "\nCLOWNTOWN: No tests!\n";
    if (empty($options['no-fun'])) {
      print <<<CLOWN
            _
           {_}
           /*\\
          /_*_\\
         {('o')}
      C{{([^*^])}}D
          [ * ]
         /  Y  \\
        _\\__|__/_
       (___/ \\___)
CLOWN
        ."\n\n";
    }

    /* Emacs' syntax highlighting gets confused by that clown and this comment
     * resets whatever state got messed up. */
    return;
  }
  $ran_tests = false;
  foreach ($results as $result) {
    // The result here will either be skipped or passed (since failed is
    // handled in print_failure.
    if ($result['status'] == 'passed') {
      $ran_tests = true;
      break;
    }
  }
  // We just had skipped tests
  if (!$ran_tests) {
    print "\nSKIP-ALOO: Only skipped tests!\n";
    if (empty($options['no-fun'])) {
      print <<<SKIPPER
                        .".
                         /  |
                        /  /
                       / ,"
           .-------.--- /
          "._ __.-/ o. o\
             "   (    Y  )
                  )     /
                 /     (
                /       Y
            .-"         |
           /  _     \    \
          /    `. ". ) /' )
         Y       )( / /(,/
        ,|      /     )
       ( |     /     /
        " \_  (__   (__
            "-._,)--._,)
SKIPPER
        ."\n\n";
    }

    /* Emacs' syntax highlighting may get confused by the skipper and this
     * rcomment esets whatever state got messed up. */
    return;
  }
  print "\nAll tests passed.\n";
  if (empty($options['no-fun'])) {
    print <<<SHIP
              |    |    |
             )_)  )_)  )_)
            )___))___))___)\
           )____)____)_____)\\
         _____|____|____|____\\\__
---------\      SHIP IT      /---------
  ^^^^^ ^^^^^^^^^^^^^^^^^^^^^
    ^^^^      ^^^^     ^^^    ^^
         ^^^^      ^^^
SHIP
      ."\n";
  }
  if (!empty($options['failure-file'])) {
    @unlink($options['failure-file']);
  }
  if (isset($options['verbose'])) {
    print_commands($tests, $options);
  }
}

function aggregate_srcloc_info($tests) {
  $all_missing_on_lhs = [];
  $all_missing_on_rhs = [];
  $all_mismatched_srcloc = [];

  foreach ($tests as $test) {
    $file = "$test.diff";
    $json_content = file_get_contents($file);
    $srcloc_info = json_decode($json_content, true);

    $mismatched_srcloc = $srcloc_info['mismatched_srcloc'];
    foreach ($mismatched_srcloc as $instr) {
      $all_mismatched_srcloc[] = $instr;
    }
    $all_mismatched_srcloc = array_unique($all_mismatched_srcloc);
    sort(&$all_mismatched_srcloc);

    $missing_on_lhs_srcloc = $srcloc_info['missing_on_lhs_srcloc'];
    foreach ($missing_on_lhs_srcloc as $instr) {
      $all_missing_on_lhs[] = $instr;
    }
    $all_missing_on_lhs = array_unique($all_missing_on_lhs);
    sort(&$all_missing_on_lhs);

    $missing_on_rhs_srcloc = $srcloc_info['missing_on_rhs_srcloc'];
    foreach ($missing_on_rhs_srcloc as $instr) {
      $all_missing_on_rhs[] = $instr;
    }
    $all_missing_on_rhs = array_unique($all_missing_on_rhs);
    sort(&$all_missing_on_rhs);
  }

  print "\nInstructions with mismatched srcloc:\n";
  print "  - " . implode(",\n  - ", $all_mismatched_srcloc) . "\n\n";
  print "Instructions with missing srcloc in HackC:\n";
  print "  - " . implode(",\n  - ", $all_missing_on_lhs) . "\n\n";
  print "Instructions with missing srcloc in HHVM:\n";
  print "  - " . implode(",\n  - ", $all_missing_on_rhs) . "\n\n";
  print "\n";
}

function print_failure($argv, $results, $options) {
  $failed = array();
  $passed = array();
  foreach ($results as $result) {
    if ($result['status'] === 'failed') {
      $failed[] = $result['name'];
    }
    if ($result['status'] === 'passed') {
      $passed[] = $result['name'];
    }
  }
  asort(&$failed);
  print "\n".count($failed)." tests failed\n";
  if (empty($options['no-fun'])) {
    // Unicode for table-flipping emoticon
    print "(\u{256F}\u{00B0}\u{25A1}\u{00B0}\u{FF09}\u{256F}\u{FE35} \u{253B}";
    print "\u{2501}\u{253B}\n";
    // TODO: Google indicates that this is some old emoji-thing relating to
    // table flipping... Maybe replace to stop other people spending time
    // trying to decipher it?
    // https://knowyourmeme.com/memes/flipping-tables
  }

  if (isset($options['srcloc'])) {
    aggregate_srcloc_info ($failed);
  } else {
    print make_header("See the diffs:").
      implode("\n", array_map(
        function($test) { return 'cat '.$test.'.diff'; },
      $failed))."\n";

    $failing_tests_file = !empty($options['failure-file'])
      ? $options['failure-file']
      : tempnam('/tmp', 'test-failures');
    file_put_contents($failing_tests_file, implode("\n", $failed)."\n");
    print make_header('For xargs, list of failures is available using:').
      'cat '.$failing_tests_file."\n";

    if (!empty($passed)) {
      $passing_tests_file = !empty($options['success-file'])
        ? $options['success-file']
        : tempnam('/tmp', 'tests-passed');
      file_put_contents($passing_tests_file, implode("\n", $passed)."\n");
      print make_header('For xargs, list of passed tests is available using:').
        'cat '.$passing_tests_file."\n";
    }

    print_commands($failed, $options);

    print
      make_header("Re-run just the failing tests:") .
      str_replace("run.php", "run", $argv[0]) . ' ' .
      implode(' ', $GLOBALS['recorded_options']) .
      sprintf(' $(cat %s)%s', $failing_tests_file, "\n");
  }
}

function port_is_listening($port) {
  $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
  return @socket_connect($socket, 'localhost', $port);
}

function find_open_port() {
  for ($i = 0; $i < 50; ++$i) {
    $port = rand(1024, 65535);
    if (!port_is_listening($port)) return $port;
  }

  error("Couldn't find an open port");
}

function start_server_proc($options, $config, $port) {
  if (isset($options['cli-server'])) {
    $cli_sock = tempnam(sys_get_temp_dir(), 'hhvm-cli-');
  } else {
    // still want to test that an unwritable socket works...
    $cli_sock = '/var/run/hhvm-cli.sock';
  }
  $threads = $options['threads'];
  $thread_option = isset($options['cli-server'])
    ? '-vEval.UnixServerWorkers='.$threads
    : '-vServer.ThreadCount='.$threads;
  $command = hhvm_cmd_impl(
    $options,
    $config,
    '-m', 'server',
    "-vServer.Port=$port",
    "-vServer.Type=proxygen",
    "-vAdminServer.Port=0",
    $thread_option,
    '-vServer.ExitOnBindFail=1',
    '-vServer.RequestTimeoutSeconds='.SERVER_TIMEOUT,
    '-vPageletServer.ThreadCount=0',
    '-vLog.UseRequestLog=1',
    '-vLog.File=/dev/null',

    // The server will unlink the temp file
    '-vEval.UnixServerPath='.$cli_sock,

    // This ensures we actually jit everything:
    '-vEval.JitRequireWriteLease=1',

    // The default test config uses a small TC but we'll be running thousands
    // of tests against the same process:
    '-vEval.JitASize=142606336',
    '-vEval.JitAProfSize=251658240',
    '-vEval.JitAColdSize=201326592',
    '-vEval.JitAFrozenSize=251658240',
    '-vEval.JitGlobalDataSize=32000000',

    // load/store counters don't work on Ivy Bridge so disable for tests
    '-vEval.ProfileHWEnable=false'
  );
  if (is_array($command)) {
    error("Can't run multi-mode tests in server mode");
  }
  if (getenv('HHVM_TEST_SERVER_LOG')) {
    echo "Starting server '$command'\n";
  }

  $descriptors = array(
    0 => array('file', '/dev/null', 'r'),
    1 => array('file', '/dev/null', 'w'),
    2 => array('file', '/dev/null', 'w'),
  );

  $proc = proc_open($command, $descriptors, &$dummy);
  if (!$proc) {
    error("Failed to start server process");
  }
  $status = proc_get_status($proc);
  $status['proc'] = $proc;
  $status['port'] = $port;
  $status['config'] = $config;
  $status['cli-socket'] = $cli_sock;
  return $status;
}

/*
 * For each config file in $configs, start up a server on a randomly-determined
 * port. Return value is an array mapping pids and config files to arrays of
 * information about the server.
 */
function start_servers($options, $configs) {
  $starting = array();
  foreach ($configs as $config) {
    $starting[] = start_server_proc($options, $config, find_open_port());
  }

  $start_time = microtime(true);
  $servers = array('pids' => array(), 'configs' => array());

  // Wait for all servers to come up.
  while (count($starting) > 0) {
    $still_starting = array();

    foreach ($starting as $server) {
      $new_status = proc_get_status($server['proc']);

      if (!$new_status['running']) {
        if ($new_status['exitcode'] === 0) {
          error("Server exited prematurely but without error");
        }

        // We lost a race. Try another port.
        if (getenv('HHVM_TEST_SERVER_LOG')) {
          echo "\n\nLost connection race on port $port. Trying another.\n\n";
        }
        $still_starting[] =
          start_server_proc($options, $server['config'], find_open_port());
      } else if (!port_is_listening($server['port'])) {
        $still_starting[] = $server;
      } else {
        $servers['pids'][$server['pid']] =& $server;
        $servers['configs'][$server['config']] =& $server;
        unset($server);
      }
    }

    $starting = $still_starting;
    $max_time = 10;
    if (microtime(true) - $start_time > $max_time) {
      error("Servers took more than $max_time seconds to come up");
    }

    // Take a short nap and try again.
    usleep(100000);
  }

  $elapsed = microtime(true) - $start_time;
  printf("Started %d servers in %.1f seconds\n\n", count($configs), $elapsed);
  return $servers;
}

function drain_queue($queue) {
  while (@msg_receive($queue, 0, &$type, 1024, &$message, true,
                      MSG_IPC_NOWAIT | MSG_NOERROR));
}

function get_num_threads($options, $tests) {
  if ($options['typechecker'] ?? false) {
    // hh_server spawns a child per CPU; things get flakey with CPU^2 forks
    return 1;
  }
  $cpus = isset($options['server']) || isset($options['cli-server'])
    ? num_cpus() * 2 : num_cpus();
  return min(count($tests), idx($options, 'threads', $cpus));
}

function runner_precheck() {
  // basic checking for runner.
  if (empty($_SERVER) || empty($_ENV)) {
    echo "Warning: \$_SERVER/\$_ENV variables not available, please check \n" .
         "your ini setting: variables_order, it should have both 'E' and 'S'\n";
  }
}

function main($argv) {
  runner_precheck();

  ini_set('pcre.backtrack_limit', PHP_INT_MAX);

  list($options, $files) = get_options($argv);
  if (isset($options['help'])) {
    error(help());
  }
  if (isset($options['list-tests'])) {
    error(list_tests($files, $options));
  }

  $tests = find_tests($files, $options);
  if (isset($options['shuffle'])) {
    shuffle($tests);
  }

  if (isset($options['repo']) && isset($options['typechecker'])) {
    error("Repo mode and typechecker mode are not compatible");
  }

  if (isset($options['hhvm-binary-path']) &&
      isset($options['typechecker'])) {
    error("Did you mean to set the hh_server binary path instead?");
  }

  if (isset($options['hhserver-binary-path']) &&
      !isset($options['typechecker'])) {
    error("hh_server binary path set, but not --typechecker");
  }

  if (isset($options['hhvm-binary-path']) &&
      isset($options['hhserver-binary-path'])) {
    error("Need to choose one of the two binaries to run");
  }

  $binary_path = "";
  $typechecker = false;
  if (isset($options['hhvm-binary-path'])) {
    check_executable($options['hhvm-binary-path'], false);
    $binary_path = realpath($options['hhvm-binary-path']);
    putenv("HHVM_BIN=" . $binary_path);
  } else if (isset($options['hhserver-binary-path'])) {
    check_executable($options['hhserver-binary-path'], true);
    $binary_path = realpath($options['hhserver-binary-path']);
    $typechecker = true;
    putenv("HH_SERVER_BIN=" . $binary_path);
  } else if (isset($options['typechecker'])) {
    $typechecker = true;
  }

  // Explicit path given by --hhvm-binary-path or --hhserver-binary-path
  // takes priority (see above)
  // Then, if an HHVM_BIN or HH_SERVER env var exists, and the file it
  // points to exists, that trumps any default hhvm / typechecker executable
  // path.
  if ($binary_path === "") {
    if (!$typechecker) {
      if (getenv("HHVM_BIN") !== false) {
        $binary_path = realpath(getenv("HHVM_BIN"));
        check_executable($binary_path, false);
      } else {
        check_for_multiple_default_binaries(false);
        $binary_path = hhvm_path();
      }
    } else {
      if (getenv("HH_SERVER_BIN") !== false) {
        $binary_path = realpath(getenv("HH_SERVER_BIN"));
        check_executable($binary_path, true);
      } else {
        check_for_multiple_default_binaries(true);
        $binary_path = hh_server_path();
      }
    }
  }

  if (isset($options['verbose'])) {
    print "You are using the binary located at: " . $binary_path . "\n";
  }

  $options['threads'] = get_num_threads($options, $tests);

  $servers = null;
  if (isset($options['server']) || isset($options['cli-server'])) {
    if (isset($options['server']) && isset($options['cli-server'])) {
      error("Server mode and CLI Server mode are mutually exclusive");
    }
    if (isset($options['repo']) || isset($options['typechecker'])) {
      error("Server mode repo tests are not supported");
    }
    $configs = array();

    /* We need to start up a separate server process for each config file
     * found. */
    foreach ($tests as $test) {
      if (!can_run_server_test($test)) continue;
      $config = find_file_for_dir(dirname($test), 'config.ini');
      if (!$config) {
        error("Couldn't find config file for $test");
      }
      $configs[$config] = $config;
    }

    $max_configs = 30;
    if (count($configs) > $max_configs) {
      error("More than $max_configs unique config files will be needed to run ".
            "the tests you specified. They may not be a good fit for server ".
            "mode. (".count($configs)." required)");
    }

    $servers = $options['servers'] = start_servers($options, $configs);
  }

  // Try to construct the buckets so the test results are ready in
  // approximately alphabetical order.
  $test_buckets = array();
  $i = 0;

  // Get the serial tests to be in their own bucket later.
  $serial_tests = serial_only_tests($tests);

  // If we have no serial tests, we can use the maximum number of allowed
  // threads for the test running. If we have some, we save one thread for
  // the serial bucket.
  $parallel_threads = count($serial_tests) > 0
                    ? $options['threads'] - 1
                    : $options['threads'];

  foreach ($tests as $test) {
    if (!in_array($test, $serial_tests)) {
      $test_buckets[$i][] = $test;
      $i = ($i + 1) % $parallel_threads;
    }
  }

  if (count($serial_tests) > 0) {
    // The last bucket is serial.
    // If the number of parallel tests didn't equal the actual number of
    // parallel threads because the number of serial tests reduced it enough,
    // then our next bucket is just $i; otherwise it is final available
    // thread. For example, we have 13 total tests which initially gave us
    // 13 parallel threads, but then we find out 3 are serial, so the parallel
    // tests would only fill 9 parallel buckets (< 12). The next one would be
    // 10 for the 3 serial. Now if we have 40 total tests which gave us 32
    // parallel threads and 4 serial tests, then all of possible parallel
    // buckets (31) would be filled regardless; so the serial bucket is what
    // would have been the last parallel thread (32).
    // $i got bumped to the next bucket at the end of the parallel test loop
    // above, so no $i++ here.
    $i = count($tests) - count($serial_tests) < $parallel_threads
       ? $i // we didn't fill all the parallel buckets, so use next one in line
       : $options['threads'] - 1; // all parallel filled; last thread; 0 indexed
    foreach ($serial_tests as $test) {
        $test_buckets[$i][] = $test;
    }
  }

  // If our total number of test buckets didn't overflow back to 0 above
  // when we % against the number of threads (because we didn't have that
  // many tests for this run), then just set the threads to how many
  // buckets we actually have to make calculations below correct.
  if (count($test_buckets) < $options['threads']) {
    $options['threads'] = count($test_buckets);
  }

  // Remember that the serial tests are also in the tests array too,
  // so they are part of the total count.
  if (!isset($options['testpilot'])) {
    print "Running ".count($tests)." tests in ".
      $options['threads']." threads (" . count($serial_tests) .
      " in serial)\n";
  }

  if (isset($options['verbose'])) {
    Status::setMode(Status::MODE_VERBOSE);
  }
  if (isset($options['testpilot'])) {
    Status::setMode(Status::MODE_TESTPILOT);
  }
  if (isset($options['record-failures'])) {
    Status::setMode(Status::MODE_RECORD_FAILURES);
  }
  Status::setUseColor(isset($options['color']) ? true : posix_isatty(STDOUT));

  Status::$key = rand();
  Status::$nofork = count($tests) == 1 && !$servers;
  if (!Status::$nofork) {
    $queue = Status::getQueue();
    drain_queue($queue);
  }

  Status::started();
  // Spawn off worker threads.
  $children = array();
  // A poor man's shared memory.
  $bad_test_files = array();
  if (Status::$nofork) {
    $bad_test_file = tempnam('/tmp', 'test-run-');
    $bad_test_files[] = $bad_test_file;
    $return_value = run($options, $test_buckets[$i], $bad_test_file);
  } else {
    for ($i = 0; $i < $options['threads']; $i++) {
      $bad_test_file = tempnam('/tmp', 'test-run-');
      $bad_test_files[] = $bad_test_file;
      $pid = pcntl_fork();
      if ($pid == -1) {
        error('could not fork');
      } else if ($pid) {
        $children[$pid] = $pid;
      } else {
        exit(run($options, $test_buckets[$i], $bad_test_file));
      }
    }

    // Fork off a child to receive messages and print status, and have the parent
    // wait for all children to exit.
    $printer_pid = pcntl_fork();
    if ($printer_pid == -1) {
      error("failed to fork");
    } else if ($printer_pid == 0) {
      msg_loop(count($tests), $queue);
      return 0;
    }

    // In case we exit in a crazy way, have the parent blow up the queue.
    // Do this here so no children inherit this.
    $kill_queue = function() { Status::killQueue(); };
    register_shutdown_function($kill_queue);
    pcntl_signal(SIGTERM, $kill_queue);
    pcntl_signal(SIGINT, $kill_queue);

    $return_value = 0;
    while (count($children) && $printer_pid != 0) {
      $pid = pcntl_wait(&$status);
      if (!pcntl_wifexited($status) && !pcntl_wifsignaled($status)) {
        error("Unexpected exit status from child");
      }

      if ($pid == $printer_pid) {
        // We should be finishing up soon.
        $printer_pid = 0;
      } else if (isset($servers['pids'][$pid])) {
        // A server crashed. Restart it.
        if (getenv('HHVM_TEST_SERVER_LOG')) {
          echo "\nServer $pid crashed. Restarting.\n";
        }
        Status::serverRestarted();
        $server =& $servers['pids'][$pid];
        $server = start_server_proc($options, $server['config'], $server['port']);

        // Unset the old $pid entry and insert the new one.
        unset($servers['pids'][$pid]);
        $servers['pids'][$server['pid']] =& $server;
        unset($server);
      } elseif (isset($children[$pid])) {
        unset($children[$pid]);
        $return_value |= pcntl_wexitstatus($status);
      } // Else, ignorable signal
    }
  }

  Status::finished();

  // Kill the server.
  if ($servers) {
    foreach ($servers['pids'] as $server) {
      proc_terminate($server['proc']);
      proc_close($server['proc']);
    }
  }

  // aggregate results
  $results = array();
  foreach ($bad_test_files as $bad_test_file) {
    $json = json_decode(file_get_contents($bad_test_file), true);
    if (!is_array($json)) {
      error(
        "\nNo JSON output was received from a test thread. ".
        "Either you killed it, or it might be a bug in the test script."
      );
    }
    $results = array_merge($results, $json);
    unlink($bad_test_file);
  }

  // print results
  if (isset($options['record-failures'])) {
    $fail_file = $options['record-failures'];
    $failed_tests = array();
    $prev_failing = array();
    if (file_exists($fail_file)) {
      $prev_failing = explode("\n", file_get_contents($fail_file));
    }

    $new_fails = 0;
    $new_passes = 0;
    foreach ($results as $r) {
      if (!isset($r['name']) || !isset($r['status'])) continue;
      $test = canonical_path($r['name']);
      $status = $r['status'];
      if ($status === 'passed' && in_array($test, $prev_failing)) {
        $new_passes++;
        continue;
      }
      if ($status !== 'failed') continue;
      if (!in_array($test, $prev_failing)) $new_fails++;
      $failed_tests[] = $test;
    }
    printf(
      "Recording %d tests as failing.\n".
      "There are %d new failing tests, and %d new passing tests.\n",
      count($failed_tests), $new_fails, $new_passes
    );
    sort(&$failed_tests);
    file_put_contents($fail_file, implode("\n", $failed_tests));
  } else if (isset($options['testpilot'])) {
    Status::say(array('op' => 'all_done', 'results' => $results));
    return $return_value;
  } else if (!$return_value) {
    print_success($tests, $results, $options);
  } else {
    print_failure($argv, $results, $options);
  }

  Status::sayColor("\nTotal time for all executed tests as run: ",
                   Status::BLUE,
                   sprintf("%.2fs\n",
                   Status::getOverallEndTime() -
                   Status::getOverallStartTime()));
  Status::sayColor("Total time for all executed tests if run serially: ",
                   Status::BLUE,
                   sprintf("%.2fs\n",
                   Status::addTestTimesSerial()));

  return $return_value;
}

exit(main($argv));
