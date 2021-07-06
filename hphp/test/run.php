<?hh
/**
* Run the test suites in various configurations.
*/

const TIMEOUT_SECONDS = 300;

// NOTE: The "HPHP_HOME" environment variable can be set (to ".../fbcode"), to
// define "hphp_home()" and (indirectly) "test_dir()".  Otherwise, we will use
// "__DIR__" as "test_dir()", and its grandparent directory for "hphp_home()"
// (unless we are testing a dso extensions).

<<__Memoize>>
function is_testing_dso_extension() {
  $home = getenv("HPHP_HOME");
  if ($home is string) {
    return false;
  }
  // detecting if we're running outside of the hhvm codebase.
  return !is_file(__DIR__."/../../hphp/test/run.php");
}

<<__Memoize>>
function hphp_home() {
  $home = getenv("HPHP_HOME");
  if ($home is string) {
    return realpath($home);
  }
  if (is_testing_dso_extension()) {
    return realpath(__DIR__);
  }
  return realpath(__DIR__."/../..");
}

<<__Memoize>>
function test_dir(): string {
  $home = getenv("HPHP_HOME");
  if ($home is string) {
    return realpath($home)."/hphp/test";
  }
  return __DIR__;
}

function get_expect_file_and_type($test, $options) {
  $types = varray[
    'expect',
    'expectf',
    'expectregex',
    'hhvm.expect',
    'hhvm.expectf',
  ];
  if (isset($options['repo'])) {
    if (file_exists($test . '.hphpc_assert')) {
      return varray[$test . '.hphpc_assert', 'expectf'];
    }
    if (file_exists($test . '.hhbbc_assert')) {
      return varray[$test . '.hhbbc_assert', 'expectf'];
    }
    foreach ($types as $type) {
      $fname = "$test.$type-repo";
      if (file_exists($fname)) {
        return varray[$fname, $type];
      }
    }
  }

  foreach ($types as $type) {
    $fname = "$test.$type";
    if (file_exists($fname)) {
      return varray[$fname, $type];
    }
  }
  return varray[null, null];
}

function multi_request_modes() {
  return varray['retranslate-all',
               'recycle-tc',
               'jit-serialize',
               'cli-server'];
}

function has_multi_request_mode($options) {
  foreach (multi_request_modes() as $option) {
    if (isset($options[$option])) return true;
  }
  return false;
}

function test_repo($options, $test) {
  if (isset($options['repo-out'])) {
    return $options['repo-out'] . '/' . str_replace('/', '.', $test) . '.repo';
  }
  return Status::getTestTmpPath($test, 'repo');
}

function jit_serialize_option(string $cmd, $test, $options, $serialize) {
  $serialized = test_repo($options, $test) . "/jit.dump";
  $cmds = explode(' -- ', $cmd, 2);
  $cmds[0] .=
    ' --count=' . ($serialize ? (int)$options['jit-serialize'] + 1 : 1) .
    " -vEval.JitSerdesFile=\"" . $serialized . "\"" .
    " -vEval.JitSerdesMode=" . ($serialize ? 'Serialize' : 'DeserializeOrFail') .
    ($serialize ? " -vEval.JitSerializeOptProfRequests=" . (int)$options['jit-serialize'] : '');
  if (isset($options['jitsample']) && $serialize) {
    $cmds[0] .= ' -vDeploymentId="' . $options['jitsample'] . '-serialize"';
  }
  return implode(' -- ', $cmds);
}

function usage() {
  $argv = \HH\global_get('argv');
  return "usage: {$argv[0]} [-m jit|interp] [-r] <test/directories>";
}

function help() {
  $argv = \HH\global_get('argv');
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
  % {$argv[0]} test/quick

  # Slow tests in interp mode:
  % {$argv[0]} -m interp test/slow

  # PHP specification tests in JIT mode:
  % {$argv[0]} test/slow/spec

  # Slow closure tests in JIT mode:
  % {$argv[0]} test/slow/closure

  # Slow closure tests in JIT mode with RepoAuthoritative:
  % {$argv[0]} -r test/slow/closure

  # Slow array tests, in RepoAuthoritative:
  % {$argv[0]} -r test/slow/array

  # Zend tests with a "z" in their name:
  % {$argv[0]} $ztestexample

  # Quick tests in JIT mode with some extra runtime options:
  % {$argv[0]} test/quick -a '-vEval.JitMaxTranslations=120 -vEval.HHIRRefcountOpts=0'

  # Quick tests in JIT mode with RepoAuthoritative and an extra compile-time option:
  % {$argv[0]} test/quick -r --compiler-args '--parse-on-demand=false'

  # All quick tests except debugger
  % {$argv[0]} -e debugger test/quick

  # All tests except those containing a string of 3 digits
  % {$argv[0]} -E '/\d{3}/' all

  # All tests whose name containing pdo_mysql
  % {$argv[0]} -i pdo_mysql -m jit -r zend

  # Print all the standard tests
  % {$argv[0]} --list-tests

  # Use a specific HHVM binary
  % {$argv[0]} -b ~/code/hhvm/hphp/hhvm/hhvm
  % {$argv[0]} --hhvm-binary-path ~/code/hhvm/hphp/hhvm/hhvm

  # Use retranslate all.  Run the test n times, then run retranslate all, then
  # run the test n more on the new code.
  % {$argv[0]} --retranslate-all 2 quick

  # Use jit-serialize.  Run the test n times, then run retranslate all, run the
  # test once more, serialize all profile data, and then restart hhvm, load the
  # serialized state and run retranslate-all before starting the test.
  % {$argv[0]} --jit-serialize  2 -r quick
EOT;
  return usage().$help;
}

function error($message) {
  print "$message\n";
  exit(1);
}

function success($message) {
  print "$message\n";
  exit(0);
}

// If a user-supplied path is provided, let's make sure we have a valid
// executable. Returns canonicanalized path or exits.
function check_executable(string $path): string {
  $rpath = realpath($path);
  if ($rpath === false || !is_executable($rpath)) {
    error("Provided HHVM executable ($path) is not an executable file.\n" .
          "If using HHVM_BIN, make sure that is set correctly.");
  }

  $output = varray[];
  $return_var = -1;
  exec($rpath . " --version 2> /dev/null", inout $output, inout $return_var);
  if (strpos(implode($output), "HipHop ") !== 0) {
    error("Provided file ($rpath) is not an HHVM executable.\n" .
          "If using HHVM_BIN, make sure that is set correctly.");
  }

  return $rpath;
}

function hhvm_binary_routes() {
  return darray[
    "buck"    => "/buck-out/gen/hphp/hhvm/hhvm",
    "cmake"   => "/hphp/hhvm"
  ];
}

function hh_codegen_binary_routes() {
  return darray[
    "buck"    => "/buck-out/bin/hphp/hack/src/hh_single_compile",
    "cmake"   => "/hphp/hack/bin"
  ];
}

// For Facebook: We have several build systems, and we can use any of them in
// the same code repo.  If multiple binaries exist, we want the onus to be on
// the user to specify a particular one because before we chose the buck one
// by default and that could cause unexpected results.
function check_for_multiple_default_binaries() {
  // Env var we use in testing that'll pick which build system to use.
  if (getenv("FBCODE_BUILD_TOOL") !== false) {
    return;
  }

  $home = hphp_home();
  $found = varray[];
  foreach (hhvm_binary_routes() as $path) {
    $abs_path = $home . $path . "/hhvm";
    if (file_exists($abs_path)) {
      $found[] = $abs_path;
    }
  }

  if (count($found) <= 1) {
    return;
  }

  $msg = "Multiple binaries exist in this repo. \n";
  foreach ($found as $bin) {
    $msg .= " - " . $bin . "\n";
  }
  $msg .= "Are you in fbcode?  If so, remove a binary \n"
    . "or use the --hhvm-binary-path option to the test runner. \n"
    . "e.g. test/run --hhvm-binary-path /path/to/binary slow\n";
  error($msg);
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
      $output = null;
      $return_var = -1;
      exec("which hhvm 2> /dev/null", inout $output, inout $return_var);
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

function unit_cache_file() {
  return Status::getTmpPathFile('unit-cache.sql');
}

function read_opts_file($file) {
  if ($file === null || !file_exists($file)) {
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
  $from_len = count($from);
  $to_len   = count($to);

  // find first non-matching dir.
  for ($d = 0; $d < $from_len; ++$d) {
    if ($d >= $to_len || $from[$d] !== $to[$d])
      break;
  }

  $relPath = vec[];

  // get number of remaining dirs in $from.
  $remaining = $from_len - $d - 1;
  if ($remaining > 0) {
    // add traversals up to first matching dir.
    while ($remaining-- > 0) $relPath[] = '..';
  } else {
    $relPath[] = '.';
  }
  while ($d < $to_len) $relPath[] = $to[$d++];
  return implode('/', $relPath);
}

function get_options($argv): (darray<string, mixed>, varray<string>) {
  // Options marked * affect test behavior, and need to be reported by list_tests.
  // Options with a trailing : take a value.
  $parameters = darray[
    '*env:' => '',
    'exclude:' => 'e:',
    'exclude-pattern:' => 'E:',
    'exclude-recorded-failures:' => 'x:',
    'include:' => 'i:',
    'include-pattern:' => 'I:',
    '*repo' => 'r',
    '*split-hphpc' => '',
    '*repo-single' => '',
    '*repo-separate' => '',
    '*repo-threads:' => '',
    '*repo-out:' => '',
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
    '*compiler-args:' => '',
    'log' => 'l',
    'failure-file:' => '',
    '*wholecfg' => '',
    '*hhas-round-trip' => '',
    '*use-internal-compiler' => '',
    'color' => 'c',
    'no-fun' => '',
    'no-skipif' => '',
    'cores' => '',
    'dump-tc' => '',
    'no-clean' => '',
    'list-tests' => '',
    '*recycle-tc:' => '',
    '*retranslate-all:' => '',
    '*jit-serialize:' => '',
    '*hhvm-binary-path:' => 'b:',
    '*vendor:' => '',
    'record-failures:' => '',
    '*hackc' => '',
    '*ignore-oids' => '',
    'jitsample:' => '',
    '*hh_single_type_check:' => '',
    'write-to-checkout' => '',
    'bespoke' => '',
    'lazyclass' => '',
    'hn' => '',
  ];
  $options = darray[];
  $files = varray[];
  $recorded = varray[];

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
            $arg == '--'.str_replace(varray[':', '*'], varray['', ''], $long)) {
          $record = substr($long, 0, 1) === '*';
          if ($record) $recorded[] = $arg;
          if (substr($long, -1, 1) == ':') {
            $value = $argv[++$i];
            if ($record) $recorded[] = $value;
          } else {
            $value = true;
          }
          $options[str_replace(varray[':', '*'], varray['', ''], $long)] = $value;
          $found = true;
          break;
        }
      }

      if (!$found) {
        error(sprintf("Invalid argument: '%s'\nSee {$argv[0]} --help", $arg));
      }
    } else {
      $files[] = $arg;
    }
  }

  \HH\global_set('recorded_options', $recorded);

  if (isset($options['repo-out']) && !is_dir($options['repo-out'])) {
    if (!mkdir($options['repo-out']) && !is_dir($options['repo-out'])) {
      echo "Unable to create repo-out dir " . $options['repo-out'] . "\n";
      exit(1);
    }
  }
  if (isset($options['hhbbc2'])) {
    $options['repo-separate'] = true;
    if (isset($options['repo']) || isset($options['repo-single'])) {
      echo "repo-single/repo and hhbbc2 are mutually exclusive options\n";
      exit(1);
    }
    if (isset($options['mode'])) {
      echo "hhbbc2 doesn't support modes; it compares hhas, doesn't run code\n";
      exit(1);
    }
  }

  if (isset($options['repo-single']) || isset($options['repo-separate'])) {
    $options['repo'] = true;
  } else if (isset($options['repo'])) {
    // if only repo was set, then it means repo single
    $options['repo-single'] = true;
  }

  if (isset($options['jit-serialize'])) {
    if (!isset($options['repo'])) {
      echo "jit-serialize only works in repo mode\n";
      exit(1);
    }
    if (isset($options['mode']) && $options['mode'] != 'jit') {
      echo "jit-serialize only works in jit mode\n";
      exit(1);
    }
  }

  if (isset($options['split-hphpc'])) {
    if (!isset($options['repo'])) {
      echo "split-hphpc only works in repo mode\n";
      exit(1);
    }
    if (!isset($options['repo-separate'])) {
      echo "split-hphpc only works in repo-separate mode\n";
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

  if (isset($options['write-to-checkout'])) {
    Status::$write_to_checkout = true;
  }

  if (isset($options['hackc']) && isset($options['use-internal-compiler'])) {
    echo "hackc and use-internal-compiler are mutually exclusive options\n";
    exit(1);
  }

  return tuple($options, $files);
}

/*
 * Return the path to $test relative to $base, or false if $base does not
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
  $attempt = canonical_path_from_base($test, test_dir());
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
  $mappage = darray[
    'quick'      => 'hphp/test/quick',
    'slow'       => 'hphp/test/slow',
    'debugger'   => 'hphp/test/server/debugger/tests',
    'http'       => 'hphp/test/server/http/tests',
    'fastcgi'    => 'hphp/test/server/fastcgi/tests',
    'zend'       => 'hphp/test/zend/good',
    'facebook'   => 'hphp/facebook/test',

    // subset of slow we run with CLI server too
    'slow_ext_hsl' => 'hphp/test/slow/ext_hsl',

    // Subsets of zend tests.
    'zend_ext'    => 'hphp/test/zend/good/ext',
    'zend_ext_am' => 'hphp/test/zend/good/ext/[a-m]*',
    'zend_ext_nz' => 'hphp/test/zend/good/ext/[n-z]*',
    'zend_Zend'   => 'hphp/test/zend/good/Zend',
    'zend_tests'  => 'hphp/test/zend/good/tests',
  ];

  $pattern = $mappage[$file] ?? null;
  if ($pattern is nonnull) {
    $pattern = hphp_home().'/'.$pattern;
    $matches = glob($pattern);
    if (count($matches) === 0) {
      error(
        "Convenience test name '$file' is recognized but does not match ".
        "any test files (pattern = '$pattern')",
      );
    }
    return $matches;
  }

  return varray[$file];
}

// Some tests have to be run together in the same test bucket, serially, one
// after other in order to avoid races and other collisions.
function serial_only_tests(varray<string> $tests): varray<string> {
  if (is_testing_dso_extension()) {
    return varray[];
  }
  // Add a <testname>.php.serial file to make your test run in the serial
  // bucket.
  $serial_tests = varray(array_filter(
    $tests,
    function($test) {
      return file_exists($test . '.serial');
    }
  ));
  return $serial_tests;
}

// NOTE: If "files" is very long, then the shell may reject the desired
// "find" command (especially because "escapeshellarg()" adds two single
// quote characters to each file), so we split "files" into chunks below.
function exec_find(mixed $files, string $extra): mixed {
  $results = varray[];
  foreach (array_chunk($files, 500) as $chunk) {
    $efa = implode(' ', array_map(escapeshellarg<>, $chunk));
    $output = shell_exec("find $efa $extra");
    foreach (explode("\n", $output) as $result) {
      // Collect the (non-empty) results, which should all be file paths.
      if ($result !== "") $results[] = $result;
    }
  }
  return $results;
}

function find_tests($files, darray $options = null): varray<string> {
  if (!$files) {
    $files = varray['quick'];
  }
  if ($files == varray['all']) {
    $files = varray['quick', 'slow', 'zend', 'fastcgi', 'http', 'debugger'];
    if (is_dir(hphp_home() . '/hphp/facebook/test')) {
      $files[] = 'facebook';
    }
  }
  $ft = varray[];
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
  $tests = exec_find(
    $files,
    "'(' " .
    "-name '*.php' " .
    "-o -name '*.hack' " .
    "-o -name '*.hackpartial' " .
    "-o -name '*.hhas' " .
    "-o -name '*.php.type-errors' " .
    "-o -name '*.hack.type-errors' " .
    "-o -name '*.hackpartial.type-errors' " .
    "')' " .
    "-not -regex '.*round_trip[.]hhas'"
  );
  if (!$tests) {
    error("Could not find any tests associated with your options.\n" .
          "Make sure your test path is correct and that you have " .
          "the right expect files for the tests you are trying to run.\n" .
          usage());
  }
  asort(inout $tests);
  $tests = varray(array_filter($tests));
  if ($options['exclude'] ?? false) {
    $exclude = $options['exclude'];
    $tests = varray(array_filter($tests, function($test) use ($exclude) {
      return (false === strpos($test, $exclude));
    }));
  }
  if ($options['exclude-pattern'] ?? false) {
    $exclude = $options['exclude-pattern'];
    $tests = varray(array_filter($tests, function($test) use ($exclude) {
      return !preg_match($exclude, $test);
    }));
  }
  if ($options['exclude-recorded-failures'] ?? false) {
    $exclude_file = $options['exclude-recorded-failures'];
    $exclude = file($exclude_file, FILE_IGNORE_NEW_LINES);
    $tests = varray(array_filter($tests, function($test) use ($exclude) {
      return (false === in_array(canonical_path($test), $exclude));
    }));
  }
  if ($options['include'] ?? false) {
    $include = $options['include'];
    $tests = varray(array_filter($tests, function($test) use ($include) {
      return (false !== strpos($test, $include));
    }));
  }
  if ($options['include-pattern'] ?? false) {
    $include = $options['include-pattern'];
    $tests = varray(array_filter($tests, function($test) use ($include) {
      return preg_match($include, $test);
    }));
  }
  return $tests;
}

function list_tests($files, $options) {
  $args = implode(' ', \HH\global_get('recorded_options'));

  // Disable escaping of test info when listing. We check if the environment
  // variable is set so we can make the change in a backwards compatible way.
  $escape_info = getenv("LISTING_NO_ESCAPE") === false;

  foreach (find_tests($files, $options) as $test) {
    $test_info = Status::jsonEncode(
        darray['args' => $args, 'name' => $test],
    );
    if ($escape_info) {
        print str_replace('\\', '\\\\', $test_info)."\n";
    } else {
        print $test_info."\n";
    }
  }
}

function find_test_ext(
  string $test,
  string $ext,
  string $configName='config',
): ?string {
  if (is_file("{$test}.{$ext}")) {
    return "{$test}.{$ext}";
  }
  return find_file_for_dir(dirname($test), "{$configName}.{$ext}");
}

function find_file_for_dir(string $dir, string $name): ?string {
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
  $file = test_dir().'/'.$name;
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

function mode_cmd($options): varray<string> {
  $repo_args = '';
  if (!isset($options['repo'])) {
    $repo_args = "-vUnitFileCache.Path=".unit_cache_file();
  }
  $interp_args = "$repo_args -vEval.Jit=0";
  $jit_args = "$repo_args -vEval.Jit=true";
  $mode = idx($options, 'mode', '');
  switch ($mode) {
    case '':
    case 'jit':
      return varray[$jit_args];
    case 'interp':
      return varray[$interp_args];
    case 'interp,jit':
      return varray[$interp_args, $jit_args];
    default:
      error("-m must be one of jit | interp | interp,jit. Got: '$mode'");
  }
}

function extra_args($options): string {
  $args = $options['args'] ?? '';

  $vendor = $options['vendor'] ?? null;
  if ($vendor !== null) {
    $args .= ' -d auto_prepend_file=';
    $args .= escapeshellarg($vendor.'/hh_autoload.php');
  }

  if (isset($options['lazyclass'])) {
    $args .= ' -vEval.EmitClassPointers=2';
    $args .= ' -vEval.ClassPassesClassname=true';
  }
  return $args;
}

function extra_compiler_args($options): string {
  return $options['compiler-args'] ?? '';
}

function hhvm_cmd_impl(
  $options,
  $config,
  $autoload_db_prefix,
  ...$extra_args
): varray<string> {
  $cmds = varray[];
  foreach (mode_cmd($options) as $mode_num => $mode) {
    $args = varray[
      hhvm_path(),
      '-c',
      $config,
      // EnableArgsInBacktraces disables most of HHBBC's DCE optimizations.
      // In order to test those optimizations (which are part of a normal prod
      // configuration) we turn this flag off by default.
      '-vEval.EnableArgsInBacktraces=false',
      '-vEval.EnableIntrinsicsExtension=true',
      '-vEval.HHIRInliningIgnoreHints=false',
      '-vEval.HHIRAlwaysInterpIgnoreHint=false',
      '-vEval.FoldLazyClassKeys=false',
      $mode,
      isset($options['wholecfg']) ? '-vEval.JitPGORegionSelector=wholecfg' : '',

      // load/store counters don't work on Ivy Bridge so disable for tests
      '-vEval.ProfileHWEnable=false',

      isset($options['use-internal-compiler']) ?  '' :
        '-vEval.HackCompilerExtractPath='
        .escapeshellarg(bin_root().'/hackc_%{schema}'),

      // use a fixed path for embedded data
      '-vEval.EmbeddedDataExtractPath='
        .escapeshellarg(bin_root().'/hhvm_%{type}_%{buildid}'),

      // Stick to a single thread for retranslate-all
      '-vEval.JitWorkerThreads=1',
      '-vEval.JitWorkerThreadsForSerdes=1',

      extra_args($options),
    ];

    if ($autoload_db_prefix !== null) {
      $args[] = '-vAutoload.DB.Path='.escapeshellarg("$autoload_db_prefix.$mode_num");
    }

    if (isset($options['hackc'])) {
      $args[] = '-vEval.HackCompilerCommand="'.hh_codegen_cmd($options).'"';
      $args[] = '-vEval.HackCompilerUseEmbedded=false';
    }

    if (isset($options['retranslate-all'])) {
      $args[] = '--count='.((int)$options['retranslate-all'] * 2);
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

    if (isset($options['dump-tc'])) {
      $args[] = '-vEval.DumpIR=1';
      $args[] = '-vEval.DumpTC=1';
    }

    if (isset($options['hh_single_type_check'])) {
      $args[] = '--hh_single_type_check='.$options['hh_single_type_check'];
    }

    if (isset($options['bespoke'])) {
      $args[] = '-vEval.BespokeArrayLikeMode=1';
      $args[] = '-vServer.APC.MemModelTreadmill=true';
    }

    if (isset($options['use-internal-compiler'])) {
      $args[] = '-vEval.HackCompilerUseCompilerPool=false';
    }

    $cmds[] = implode(' ', array_merge($args, $extra_args));
  }
  return $cmds;
}

function repo_separate($options, $test) {
  return isset($options['repo-separate']) &&
         !file_exists($test . ".hhbbc_opts");
}

// Return the command and the env to run it in.
function hhvm_cmd(
  $options,
  $test,
  $test_run = null,
  $is_temp_file = false
): (varray<string>, darray<string, mixed>) {
  $test_run ??= $test;
  // hdf support is only temporary until we fully migrate to ini
  // Discourage broad use.
  $hdf_suffix = ".use.for.ini.migration.testing.only.hdf";
  $hdf = file_exists($test.$hdf_suffix)
       ? '-c ' . $test . $hdf_suffix
       : "";
  $extra_opts = read_opts_file(find_test_ext($test, 'opts'));
  if (isset($options['hn'])) {
    $extra_opts = read_opts_file("$test.hn_opts")." ".$extra_opts;
  }
  $cmds = hhvm_cmd_impl(
    $options,
    find_test_ext($test, 'ini'),
    Status::getTestTmpPath($test, 'autoloadDB'),
    $hdf,
    find_debug_config($test, 'hphpd.ini'),
    $extra_opts,
    $is_temp_file ? " --temp-file" : "",
    '--file',
    escapeshellarg($test_run),
  );

  $cmd = "";

  if (file_exists($test.'.verify')) {
    $cmd .= " -m verify";
  }

  if (isset($options['cli-server'])) {
    $config = find_file_for_dir(dirname($test), 'config.ini');
    $socket = $options['servers']['configs'][$config]->server['cli-socket'];
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
    $repo_suffix = repo_separate($options, $test) ? 'hhbbc' : 'hhbc';

    $program = isset($options['hackc']) ? "hackc" : "hhvm";
    $hhbbc_repo = '"' . test_repo($options, $test) . "/$program.$repo_suffix\"";
    $cmd .= ' -vRepo.Authoritative=true';
    $cmd .= " -vRepo.Path=$hhbbc_repo";
  }

  if (isset($options['jitsample'])) {
    $cmd .= ' -vDeploymentId="' . $options['jitsample'] . '"';
    $cmd .= ' --instance-id="' . $test . '"';
    $cmd .= ' -vEval.JitSampleRate=1';
    $cmd .= " -vScribe.Tables.hhvm_jit.include.*=instance_id";
    $cmd .= " -vScribe.Tables.hhvm_jit.include.*=deployment_id";
  }

  $env = $_ENV;
  $env['LC_ALL'] = 'C';

  // Apply the --env option
  if (isset($options['env'])) {
    foreach (explode(",", $options['env']) as $arg) {
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

  foreach ($cmds as $idx => $_) {
    $cmds[$idx] .= $cmd;
  }

  return tuple($cmds, $env);
}

function hphp_cmd($options, $test, $program): string {
  // Transform extra_args like "-vName=Value" into "-vRuntime.Name=Value".
  $extra_args = preg_replace("/(^-v|\s+-v)\s*/", "$1Runtime.", extra_args($options));

  $compiler_args = extra_compiler_args($options);
  if (isset($options['hackc'])) {
    $hh_single_compile = hh_codegen_path();
    $compiler_args = implode(" ", varray[
      '-vRuntime.Eval.HackCompilerUseEmbedded=false',
      "-vRuntime.Eval.HackCompilerInheritConfig=true",
      "-vRuntime.Eval.HackCompilerCommand=\"{$hh_single_compile} --daemon --dump-symbol-refs\"",
      $compiler_args
    ]);
  }

  $hdf_suffix = ".use.for.ini.migration.testing.only.hdf";
  $hdf = file_exists($test.$hdf_suffix)
       ? '-c ' . $test . $hdf_suffix
       : "";

  if ($hdf !== "") {
    $contents = file_get_contents($test.$hdf_suffix);
    if (strpos($contents, '{PWD}') !== false) {
      $test_hdf = tempnam('/tmp', $test).$hdf_suffix;
      file_put_contents($test_hdf,
                        str_replace('{PWD}', dirname($test), $contents));
      $hdf = " -c $test_hdf";
    }
  }

  return implode(" ", varray[
    hphpc_path($options),
    '--hphp',
    '-vUseHHBBC='. (repo_separate($options, $test) ? 'false' : 'true'),
    '--config',
    find_test_ext($test, 'ini', 'hphp_config'),
    $hdf,
    '-vRuntime.ResourceLimit.CoreFileSize=0',
    '-vRuntime.Eval.EnableIntrinsicsExtension=true',
    '-vRuntime.Eval.EnableArgsInBacktraces=true',
    '-vRuntime.Eval.FoldLazyClassKeys=false',
    isset($options['use-internal-compiler']) ?  '' :
      '-vRuntime.Eval.HackCompilerExtractPath='
      .escapeshellarg(bin_root().'/hackc_%{schema}'),
    '-vParserThreadCount=' . ($options['repo-threads'] ?? 1),
    '--nofork=1 -thhbc -l1 -k1',
    '-o "' . test_repo($options, $test) . '"',
    "--program $program.hhbc \"$test\"",
    "-vRuntime.UnitFileCache.Path=".unit_cache_file(),
    $extra_args,
    $compiler_args,
    read_opts_file("$test.hphp_opts"),
  ]);
}

function hphpc_path($options) {
  if (isset($options['split-hphpc'])) {
    $file = "";
    $file = bin_root().'/hphpc';

    if (!is_file($file)) {
      error("$file doesn't exist. Did you forget to build first?");
    }
    return rel_path($file);
  } else {
    return hhvm_path();
  }
}

function hhbbc_cmd($options, $test, $program) {
  $test_repo = test_repo($options, $test);
  return implode(" ", varray[
    hphpc_path($options),
    '--hhbbc',
    '--no-logging',
    '--no-cores',
    '--parallel-num-threads=' . ($options['repo-threads'] ?? 1),
    '--hack-compiler-extract-path='
      .escapeshellarg(bin_root().'/hackc_%{schema}'),
    read_opts_file("$test.hhbbc_opts"),
    "-o \"$test_repo/$program.hhbbc\" \"$test_repo/$program.hhbc\"",
  ]);
}

// Execute $cmd and return its output, including any stacktrace.log
// file it generated.
function exec_with_stack($cmd) {
  $pipes = null;
  $proc = proc_open($cmd,
                    darray[0 => varray['pipe', 'r'],
                          1 => varray['pipe', 'w'],
                          2 => varray['pipe', 'w']], inout $pipes);
  fclose($pipes[0]);
  $s = '';
  $all_selects_failed=true;
  $end = microtime(true) + TIMEOUT_SECONDS;
  $timedout = false;
  while (true) {
    $now = microtime(true);
    if ($now >= $end) break;
    $read = varray[$pipes[1], $pipes[2]];
    $write = null;
    $except = null;
    $available = @stream_select(
      inout $read,
      inout $write,
      inout $except,
      (int)($end - $now),
    );
    if ($available === false) {
      usleep(1000);
      $s .= "select failed:\n" . print_r(error_get_last(), true);
      continue;
    }
    $all_selects_failed=false;
    if ($available === 0) continue;
    // var_dump($read);
    foreach ($read as $pipe) {
      $t = fread($pipe, 4096);
      // var_dump($t);
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
      $output = null;
      $return_var = -1;
      exec('pkill -P ' . $status['pid'] . ' 2> /dev/null', inout $output, inout $return_var);
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
  if (
    !$status['exitcode'] &&
    !preg_match('/\\b(error|exception|fatal)\\b/', $s)
  ) {
    return true;
  }
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
  if ($result === true && repo_separate($options, $test)) {
    $hhbbc = hhbbc_cmd($options, $test, $program);
    $result = exec_with_stack($hhbbc);
  }
  if ($result === true) return true;
  Status::writeDiff($test, $result);
}


// Minimal support for sending messages between processes over named pipes.
//
// Non-buffered pipe writes of up to 512 bytes (PIPE_BUF) are atomic.
//
// Packet format:
//   8 byte zero-padded hex pid
//   4 byte zero-padded hex type
//   4 byte zero-padded hex body size
//   N byte string body
//
// NOTE: The first call to "getInput()" or "getOutput()" in any process will
// block until some other process calls the other method.
//
class Queue {
  // The path to the FIFO, until destroyed.
  private ?string $path = null;

  // TODO: Use proper types.
  private mixed $input = null;
  private mixed $output = null;

  // Pipes writes are atomic up to 512 bytes (up to 4096 bytes on linux),
  // and we use a 16 byte header, leaving this many bytes available for
  // each chunk of "body" (see "$partials").
  const int CHUNK = 512 - 16;

  // If a message "body" is larger than CHUNK bytes, then writers must break
  // it into chunks, and send all but the last chunk with type 0.  The reader
  // collects those chunks in this Map (indexed by pid), until the final chunk
  // is received, and the chunks can be reassembled.
  private Map<int, Vector<string>> $partials = Map {};


  // NOTE: Only certain directories support "posix_mkfifo()".
  public function __construct(?string $dir = null): void {
    $path = \tempnam($dir ?? \sys_get_temp_dir(), "queue.mkfifo.");
    \unlink($path);
    if (!\posix_mkfifo($path, 0700)) {
      throw new \Exception("Failed to create FIFO at '$path'");
    }
    $this->path = $path;
  }

  private function getInput(): mixed {
    $input = $this->input;
    if ($input is null) {
      $path = $this->path;
      if ($path is null) {
        throw new \Exception("Missing FIFO path");
      }
      $input = \fopen($path, "r");
      $this->input = $input;
    }
    return $input;
  }

  private function getOutput(): mixed {
    $output = $this->output;
    if ($output is null) {
      $path = $this->path;
      if ($path is null) {
        throw new \Exception("Missing FIFO path");
      }
      $output = \fopen($path, "a");
      $this->output = $output;
    }
    return $output;
  }

  private function validate(int $pid, int $type, int $blen): void {
    if ($pid < 0 || $pid >= (1 << 22)) {
      throw new \Exception("Illegal pid $pid");
    }
    if ($type < 0 || $type >= 0x10000) {
      throw new \Exception("Illegal type $type");
    }
    if ($blen < 0 || $blen > static::CHUNK) {
      throw new \Exception("Illegal blen $blen");
    }
  }

  // Read one packet header or body.
  private function read(int $n): string {
    $input = $this->getInput();
    $result = "";
    while (\strlen($result) < $n) {
      $r = fread($input, $n - \strlen($result));
      if ($r is string) {
        $result .= $r;
      } else {
        throw new \Exception("Failed to read $n bytes");
      }
    }
    return $result;
  }

  // Receive one raw message (pid, type, body).
  public function receive(): (int, int, string) {
    $type = null;
    $body = "";
    while (true) {
      $header = $this->read(16);
      $pid = intval(substr($header, 0, 8) as string, 16);
      $type = intval(substr($header, 8, 4) as string, 16);
      $blen = intval(substr($header, 12, 4) as string, 16);
      $this->validate($pid, $type, $blen);
      $body = $this->read($blen);
      if ($type === 0) {
        $this->partials[$pid] ??= Vector {};
        $this->partials[$pid][] = $body;
      } else {
        $chunks = $this->partials[$pid] ?? null;
        if ($chunks is nonnull) {
          $chunks[] = $body;
          $body = \join("", $chunks);
          unset($this->partials[$pid]);
        }
        return tuple($pid, $type, $body);
      }
    }
  }

  // Receive one message (pid, type, message).
  // Note that the raw body is processed using "unserialize()".
  public function receiveMessage(): (int, int, mixed) {
    list($pid, $type, $body) = $this->receive();
    $msg = unserialize($body);
    return tuple($pid, $type, $msg);
  }

  private function write(int $pid, int $type, string $body): void {
    $output = $this->getOutput();
    $blen = \strlen($body);
    $this->validate($pid, $type, $blen);
    $packet = sprintf("%08x%04x%04x%s", $pid, $type, $blen, $body);
    $n = \strlen($packet);
    if ($n !== 16 + $blen) {
      throw new \Exception("Illegal packet");
    }
    // NOTE: Hack's "fwrite()" is never buffered, which is especially
    // critical for pipe writes, to ensure that they are actually atomic.
    // See the documentation for "PlainFile::writeImpl()".  But just in
    // case, we add an explicit "fflush()" below.
    $bytes_out = fwrite($output, $packet, $n);
    if ($bytes_out !== $n) {
      throw new \Exception(
        "Failed to write $n bytes; only $bytes_out were written"
      );
    }
    fflush($output);
  }

  // Send one raw message.
  public function send(int $type, string $body): void {
    $pid = \posix_getpid();
    $blen = \strlen($body);
    $chunk = static::CHUNK;
    if ($blen > $chunk) {
      for ($i = 0; $i + $chunk < $blen; $i += $chunk) {
        $this->write($pid, 0, \substr($body, $i, $chunk) as string);
      }
      $this->write($pid, $type, \substr($body, $i) as string);
    } else {
      $this->write($pid, $type, $body);
    }
  }

  // Send one message.
  // Note that the raw body is computed using "serialize()".
  public function sendMessage(int $type, mixed $msg): void {
    $body = serialize($msg);
    $this->send($type, $body);
  }

  function destroy(): void {
    if ($this->input is nonnull) {
      fclose($this->input);
      $this->input = null;
    }
    if ($this->output is nonnull) {
      fclose($this->output);
      $this->output = null;
    }
    if ($this->path is nonnull) {
      \unlink($this->path);
      $this->path = null;
    }
  }
}

enum TempDirRemove: int {
  ALWAYS = 0;
  ON_RUN_SUCCESS = 1;
  NEVER = 2;
}

final class Status {
  private static $results = varray[];
  private static $mode = 0;

  private static $use_color = false;

  public static $nofork = false;
  private static ?Queue $queue = null;
  private static $killed = false;
  public static TempDirRemove $temp_dir_remove = TempDirRemove::ALWAYS;
  private static int $return_value = 255;

  private static $overall_start_time = 0;
  private static $overall_end_time = 0;

  private static $tmpdir = "";
  public static $write_to_checkout = false;

  public static $passed = 0;
  public static $skipped = 0;
  public static $skip_reasons = darray[];
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

  public static function createTmpDir(): void {
    $parent = sys_get_temp_dir();
    if (substr($parent, -1) !== "/") {
      $parent .= "/";
    }
    self::$tmpdir = HH\Lib\_Private\_OS\mkdtemp($parent . 'hphp-test-XXXXXX');
  }

  public static function getRunTmpDir(): string {
    return self::$tmpdir;
  }

  // Return a path in the run tmpdir that's unique to this test and ext.
  // Remember to teach clean_intermediate_files to clean up all the exts you use
  public static function getTestTmpPath(string $test, string $ext): string {
    return self::$tmpdir . '/' . $test . '.' . $ext;
  }

  public static function getTmpPathFile(string $filename): string {
    return self::$tmpdir . '/' . $filename;
  }

  // Similar to getTestTmpPath, but if we're run with --write-to-checkout
  // then we put the files next to the test instead of in the tmpdir.
  public static function getTestOutputPath(string $test, string $ext): string {
    if (self::$write_to_checkout) {
      return "$test.$ext";
    }
    return static::getTestTmpPath($test, $ext);
  }

  public static function createTestTmpDir(string $test): string {
    $test_temp_dir = self::getTestTmpPath($test, 'tmpdir');
    @mkdir($test_temp_dir, 0777, true);
    return $test_temp_dir;
  }

  public static function writeDiff(string $test, string $diff): void {
    $path = Status::getTestOutputPath($test, 'diff');
    @mkdir(dirname($path), 0777, true);
    file_put_contents($path, $diff);
  }

  public static function diffForTest(string $test): string {
    $diff = @file_get_contents(Status::getTestOutputPath($test, 'diff'));
    return $diff === false ? '' : $diff;
  }

  public static function removeDirectory($dir) {
    $files = scandir($dir);
    foreach ($files as $file) {
      if ($file == '.' || $file == '..') {
        continue;
      }
      $path = $dir . "/" . $file;
      if (is_dir($path)) {
        self::removeDirectory($path);
      } else {
        unlink($path);
      }
    }
    rmdir($dir);
  }

  // This is similar to removeDirectory but it only removes empty directores
  // and won't enter directories whose names end with '.tmpdir'. This allows
  // us to clean up paths like test/quick/vec in our run's temporary directory
  // if all the tests in them passed, but it leaves test tmpdirs of failed
  // tests (that we didn't remove with clean_intermediate_files because the
  // test failed) and directores under them alone even if they're empty.
  public static function removeEmptyTestParentDirs($dir): bool {
    $is_now_empty = true;
    $files = scandir($dir);
    foreach ($files as $file) {
      if ($file == '.' || $file == '..') {
        continue;
      }
      if (strrpos($file, '.tmpdir') === (strlen($file) - strlen('.tmpdir'))) {
        $is_now_empty = false;
        continue;
      }
      $path = $dir . "/" . $file;
      if (!is_dir($path)) {
        $is_now_empty = false;
        continue;
      }
      if (self::removeEmptyTestParentDirs($path)) {
        rmdir($path);
      } else {
        $is_now_empty = false;
      }
    }
    return $is_now_empty;
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

  public static function addTestTimesSerial($results) {
    $time = 0.0;
    foreach ($results as $result) {
      $time += $result['time'];
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
    self::send(self::MSG_STARTED, null);
    self::$overall_start_time = microtime(true);
  }

  public static function finished(int $return_value) {
    self::$overall_end_time = microtime(true);
    self::$return_value = $return_value;
    self::send(self::MSG_FINISHED, null);
  }

  public static function destroy(): void {
    if (!self::$killed) {
      self::$killed = true;
      if (self::$queue !== null) {
        self::$queue->destroy();
        self::$queue = null;
      }
      switch (self::$temp_dir_remove) {
        case TempDirRemove::NEVER:
          break;
        case TempDirRemove::ON_RUN_SUCCESS:
          if (self::$return_value !== 0) {
            self::removeEmptyTestParentDirs(self::$tmpdir);
            break;
          }
          // FALLTHROUGH
        case TempDirRemove::ALWAYS:
          self::removeDirectory(self::$tmpdir);
      }
    }
  }

  public static function destroyFromSignal($_signo): void {
    self::destroy();
  }

  public static function registerCleanup(bool $no_clean) {
    if (self::getMode() === self::MODE_TESTPILOT ||
        self::getMode() === self::MODE_RECORD_FAILURES) {
      self::$temp_dir_remove = TempDirRemove::ALWAYS;
    } else if ($no_clean) {
      self::$temp_dir_remove = TempDirRemove::NEVER;
    } else {
      self::$temp_dir_remove = TempDirRemove::ON_RUN_SUCCESS;
    }
    register_shutdown_function(self::destroy<>);
    pcntl_signal(SIGTERM, self::destroyFromSignal<>);
    pcntl_signal(SIGINT, self::destroyFromSignal<>);
  }

  public static function serverRestarted() {
    self::send(self::MSG_SERVER_RESTARTED, null);
  }

  public static function pass($test, $time, $stime, $etime) {
    self::$results[] = darray['name' => $test,
                             'status' => 'passed',
                             'start_time' => $stime,
                             'end_time' => $etime,
                             'time' => $time];
    self::send(self::MSG_TEST_PASS, vec[$test, $time, $stime, $etime]);
  }

  public static function skip($test, $reason, $time, $stime, $etime) {
    self::$results[] = darray[
      'name' => $test,
      /* testpilot needs a positive response for every test run, report
       * that this test isn't relevant so it can silently drop. */
      'status' => self::getMode() === self::MODE_TESTPILOT
        ? 'not_relevant'
        : 'skipped',
      'start_time' => $stime,
      'end_time' => $etime,
      'time' => $time,
    ];
    self::send(self::MSG_TEST_SKIP,
               vec[$test, $reason, $time, $stime, $etime]);
  }

  public static function fail($test, $time, $stime, $etime, $diff) {
    self::$results[] = darray[
      'name' => $test,
      'status' => 'failed',
      'details' => self::utf8Sanitize($diff),
      'start_time' => $stime,
      'end_time' => $etime,
      'time' => $time
    ];
    self::send(self::MSG_TEST_FAIL, vec[$test, $time, $stime, $etime]);
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
        list($test, $time, $stime, $etime) = $message;
        switch (Status::getMode()) {
          case Status::MODE_NORMAL:
            if (!Status::hasCursorControl()) {
              Status::sayColor(Status::GREEN, '.');
            }
            break;

          case Status::MODE_VERBOSE:
            Status::sayColor("$test ", Status::GREEN,
                             sprintf("passed (%.2fs)\n", $time));
            break;

          case Status::MODE_TESTPILOT:
            Status::sayTestpilot($test, 'passed', $stime, $etime, $time);
            break;

          case Status::MODE_RECORD_FAILURES:
            break;
        }
        break;

      case Status::MSG_TEST_SKIP:
        self::$skipped++;
        list($test, $reason, $time, $stime, $etime) = $message;
        self::$skip_reasons[$reason] ??= 0;
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
              Status::sayColor(" - reason: $reason");
            }
            Status::sayColor(sprintf(" (%.2fs)\n", $time));
            break;

          case Status::MODE_TESTPILOT:
            Status::sayTestpilot($test, 'not_relevant', $stime, $etime, $time);
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
            $diff = Status::diffForTest($test);
            Status::sayColor(Status::RED, "\nFAILED",
                             ": $test\n$diff\n");
            break;

          case Status::MODE_VERBOSE:
            Status::sayColor("$test ", Status::RED,
                             sprintf("FAILED (%.2fs)\n", $time));
            break;

          case Status::MODE_TESTPILOT:
            Status::sayTestpilot($test, 'failed', $stime, $etime, $time);
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
    self::getQueue()->sendMessage($type, $msg);
  }

  /**
   * Takes a variable number of string arguments. If color output is enabled
   * and any one of the arguments is preceded by an integer (see the color
   * constants above), that argument will be given the indicated color.
   */
  public static function sayColor(...$args) {
    $n = count($args);
    for ($i = 0; $i < $n;) {
      $color = null;
      $str = $args[$i++];
      if (is_integer($str)) {
        $color = $str;
        if (self::$use_color) {
          print "\033[0;{$color}m";
        }
        $str = $args[$i++];
      }

      print $str;

      if (self::$use_color && !is_null($color)) {
        print "\033[0m";
      }
    }
  }

  public static function sayTestpilot($test, $status, $stime, $etime, $time) {
    $start = darray['op' => 'start', 'test' => $test];
    $end = darray['op' => 'test_done', 'test' => $test, 'status' => $status,
                 'start_time' => $stime, 'end_time' => $etime, 'time' => $time];
    if ($status == 'failed') {
      $end['details'] = self::utf8Sanitize(Status::diffForTest($test));
    }
    self::say($start, $end);
  }

  public static function getResults() {
    return self::$results;
  }

  /** Output is in the format expected by JsonTestRunner. */
  public static function say(...$args) {
    $data = array_map(
      $row ==> self::jsonEncode($row) . "\n",
      $args
    );
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

  <<__Memoize>>
  public static function getSTTY() {
    $descriptorspec = darray[1 => varray["pipe", "w"], 2 => varray["pipe", "w"]];
    $pipes = null;
    $process = proc_open(
      'stty -a', $descriptorspec, inout $pipes, null, null,
      darray['suppress_errors' => true]
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
      if (self::$killed) error("Killed!");
      self::$queue = new Queue(self::$tmpdir);
    }
    return self::$queue;
  }
}

function clean_intermediate_files($test, $options) {
  if (isset($options['no-clean'])) {
    return;
  }
  if (isset($options['write-to-checkout'])) {
    // in --write-to-checkout mode, normal test output goes next to the test
    $exts = varray[
      'out',
      'diff',
    ];
    foreach ($exts as $ext) {
      $file = "$test.$ext";
      if (file_exists($file)) {
        unlink($file);
      }
    }
  }
  $tmp_exts = varray[
    // normal test output goes here by default
    'out',
    'diff',
    // scratch directory the test may write to
    'tmpdir',
    // tests in --hhas-round-trip mode
    'round_trip.hhas',
    // tests in --hhbbc2 mode
    'before.round_trip.hhas',
    'after.round_trip.hhas',
    // temporary autoloader DB and associated cruft
    // We have at most two modes for now - see hhvm_cmd_impl
    'autoloadDB.0',
    'autoloadDB.0-journal',
    'autoloadDB.0-shm',
    'autoloadDB.0-wal',
    'autoloadDB.1',
    'autoloadDB.1-journal',
    'autoloadDB.1-shm',
    'autoloadDB.1-wal',
  ];
  foreach ($tmp_exts as $ext) {
    $file = Status::getTestTmpPath($test, $ext);
    if (is_dir($file)) {
      Status::removeDirectory($file);
    } else if (file_exists($file)) {
      unlink($file);
    }
  }
  // repo mode uses a directory that may or may not be in the run's tmpdir
  $repo = test_repo($options, $test);
  if (is_dir($repo)) {
    Status::removeDirectory($repo);
  }
}

function child_main(
  darray<string, mixed> $options,
  varray<string> $tests,
  string $json_results_file,
): int {
  foreach ($tests as $test) {
    run_and_log_test($options, $test);
  }
  file_put_contents($json_results_file, json_encode(Status::getResults()));
  foreach (Status::getResults() as $result) {
    if ($result['status'] == 'failed') {
      return 1;
    }
  }
  return 0;
}

/**
 * The runif feature is similar in spirit to skipif, but instead of allowing
 * one to run arbitrary code it can only skip based on pre-defined reasons
 * understood by the test runner.
 *
 * The .runif file should consist of one or more lines made up of words
 * separated by spaces, optionally followed by a comment starting with #.
 * Empty lines (or lines with only comments) are ignored. The first word
 * determines the interpretation of the rest. The .runif file will allow the
 * test to run if all the non-empty lines 'match'.
 *
 * Currently supported operations:
 *   os [not] <os_name> # matches if we are (or are not) on the named OS
 *   file <path> # matches if the file at the (possibly relative) path exists
 *   euid [not] root # matches if we are (or are not) running as root (euid==0)
 *   extension <extension_name> # matches if the named extension is available
 *   function <function_name> # matches if the named function is available
 *   class <class_name> # matches if the named class is available
 *   method <class_name> <method name> # matches if the method is available
 *   const <constant_name> # matches if the named constant is available
 *   # matches if any named locale is available for the named LC_* category
 *   locale LC_<something> <locale name>[ <another locale name>]
 *
 * Several functions in this implementation return RunifResult. Valid sets of
 * keys are:
 *   valid, error # valid will be false
 *   valid, match # valid will be true, match will be true
 *   valid, match, skip_reason # valid will be true, match will be false
 */
type RunifResult = shape(
  'valid' => bool, // was the runif file valid
  ?'error' => string, // if !valid, what was the problem
  ?'match' => bool, // did the line match/did all the lines in the file match
  ?'skip_reason' => string, // if !match, the skip reason to use
);

<<__Memoize>> function runif_canonical_os(): string {
  if (PHP_OS === 'Linux' || PHP_OS === 'Darwin') return PHP_OS;
  if (substr(PHP_OS, 0, 3) === 'WIN') return 'WIN';
  invariant_violation('add proper canonicalization for your OS');
}

function runif_known_os(string $match_os): bool {
  switch ($match_os) {
    case 'Linux':
    case 'Darwin':
    case 'WIN':
      return true;
    default:
      return false;
  }
}

function runif_os_matches(varray<string> $words): RunifResult {
  if (count($words) === 2) {
    if ($words[0] !== 'not') {
      return shape('valid' => false, 'error' => "malformed 'os' match");
    }
    $match_os = $words[1];
    $invert = true;
  } else if (count($words) === 1) {
    $match_os = $words[0];
    $invert = false;
  } else {
    return shape('valid' => false, 'error' => "malformed 'os' match");
  }
  if (!runif_known_os($match_os)) {
    return shape('valid' => false, 'error' => "unknown os '$match_os'");
  }
  $matches = (runif_canonical_os() === $match_os);
  if ($matches !== $invert) return shape('valid' => true, 'match' => true);
  return shape(
    'valid' => true,
    'match' => false,
    'skip_reason' => 'skip-runif-os-' . implode('-', $words)
  );
}

function runif_file_matches(varray<string> $words): RunifResult {
  /* This implementation has a trade-off. On the one hand, we could get more
   * accurate results if we do the check in a process with the same configs as
   * the test via runif_test_for_feature (e.g. if config differences make a
   * file we can see invisible to the test). However, this check was added to
   * skip tests where the test configs depend on a file that may be absent, in
   * which case hhvm configured the same way as the test cannot run. By doing
   * the check ourselves we can successfully skip such tests.
   */
  if (count($words) !== 1) {
    return shape('valid' => false, 'error' => "malformed 'file' match");
  }
  if (file_exists($words[0])) {
    return shape('valid' => true, 'match' => true);
  }
  return shape(
    'valid' => true,
    'match' => false,
    'skip_reason' => 'skip-runif-file',
  );
}

function runif_test_for_feature(
  darray<string, mixed> $options,
  string $test,
  string $bool_expression,
): bool {
  $tmp = tempnam(sys_get_temp_dir(), 'test-run-runif-');
  file_put_contents(
    $tmp,
    "<?hh\n" .
      "<<__EntryPoint>> function main(): void {\n" .
      "  echo ($bool_expression) as bool ? 'PRESENT' : 'ABSENT';\n" .
      "}\n",
  );

  // Run the check in non-repo mode to avoid building the repo (same features
  // should be available). Pick the mode arbitrarily for the same reason.
  $options_without_repo = $options;
  unset($options_without_repo['repo']);
  list($hhvm, $_) = hhvm_cmd($options_without_repo, $test, $tmp, true);
  $hhvm = $hhvm[0];
  // Remove any --count <n> from the command
  $hhvm = preg_replace('/ --count[ =]\d+/', '', $hhvm);
  // some tests set open_basedir to a restrictive value, override to permissive
  $hhvm .= ' -dopen_basedir= ';

  $result = shell_exec("$hhvm 2>&1");
  invariant ($result !== false, 'shell_exec in runif_test_for_feature failed');
  $result = trim($result);
  if ($result === 'ABSENT') return false;
  if ($result === 'PRESENT') return true;
  invariant_violation(
    "unexpected output from shell_exec in runif_test_for_feature: '$result'"
  );
}

function runif_euid_matches(
  darray<string, mixed> $options,
  string $test,
  varray<string> $words,
): RunifResult {
  if (count($words) === 2) {
    if ($words[0] !== 'not' || $words[1] !== 'root') {
      return shape('valid' => false, 'error' => "malformed 'euid' match");
    }
    $invert = true;
  } else if (count($words) === 1) {
    if ($words[0] !== 'root') {
      return shape('valid' => false, 'error' => "malformed 'euid' match");
    }
    $invert = false;
  } else {
    return shape('valid' => false, 'error' => "malformed 'euid' match");
  }
  $matches = runif_test_for_feature($options, $test, 'posix_geteuid() === 0');
  if ($matches !== $invert) return shape('valid' => true, 'match' => true);
  return shape(
    'valid' => true,
    'match' => false,
    'skip_reason' => 'skip-runif-euid-' . implode('-', $words)
  );
}

function runif_extension_matches(
  darray<string, mixed> $options,
  string $test,
  varray<string> $words,
): RunifResult {
  if (count($words) !== 1) {
    return shape('valid' => false, 'error' => "malformed 'extension' match");
  }
  if (runif_test_for_feature($options, $test, "extension_loaded('{$words[0]}')")) {
    return shape('valid' => true, 'match' => true);
  }
  return shape(
    'valid' => true,
    'match' => false,
    'skip_reason' => 'skip-runif-extension-' . $words[0]
  );
}

function runif_function_matches(
  darray<string, mixed> $options,
  string $test,
  varray<string> $words,
): RunifResult {
  if (count($words) !== 1) {
    return shape('valid' => false, 'error' => "malformed 'function' match");
  }
  if (runif_test_for_feature($options, $test, "function_exists('{$words[0]}')")) {
    return shape('valid' => true, 'match' => true);
  }
  return shape(
    'valid' => true,
    'match' => false,
    'skip_reason' => 'skip-runif-function-' . $words[0]
  );
}

function runif_class_matches(
  darray<string, mixed> $options,
  string $test,
  varray<string> $words,
): RunifResult {
  if (count($words) !== 1) {
    return shape('valid' => false, 'error' => "malformed 'class' match");
  }
  if (runif_test_for_feature($options, $test, "class_exists('{$words[0]}')")) {
    return shape('valid' => true, 'match' => true);
  }
  return shape(
    'valid' => true,
    'match' => false,
    'skip_reason' => 'skip-runif-class-' . $words[0]
  );
}

function runif_method_matches(
  darray<string, mixed> $options,
  string $test,
  varray<string> $words,
): RunifResult {
  if (count($words) !== 2) {
    return shape('valid' => false, 'error' => "malformed 'method' match");
  }
  if (runif_test_for_feature($options, $test,
                             "method_exists('{$words[0]}', '{$words[1]}')")) {
    return shape('valid' => true, 'match' => true);
  }
  return shape(
    'valid' => true,
    'match' => false,
    'skip_reason' => 'skip-runif-method-' . $words[0] . '-' . $words[1],
  );
}

function runif_const_matches(
  darray<string, mixed> $options,
  string $test,
  varray<string> $words,
): RunifResult {
  if (count($words) !== 1) {
    return shape('valid' => false, 'error' => "malformed 'const' match");
  }
  if (runif_test_for_feature($options, $test, "defined('{$words[0]}')")) {
    return shape('valid' => true, 'match' => true);
  }
  return shape(
    'valid' => true,
    'match' => false,
    'skip_reason' => 'skip-runif-const-' . $words[0]
  );
}

function runif_locale_matches(
  darray<string, mixed> $options,
  string $test,
  varray<string> $words,
): RunifResult {
  if (count($words) < 2) {
    return shape('valid' => false, 'error' => "malformed 'locale' match");
  }
  $category = array_shift(inout $words);
  if (!preg_match('/^LC_[A-Z]+$/', $category)) {
    return shape('valid' => false, 'error' => "bad locale category '$category'");
  }
  $locale_args = implode(', ', array_map($word ==> "'$word'", $words));
  $matches = runif_test_for_feature(
    $options,
    $test,
    "defined('$category') && (false !== setlocale($category, $locale_args))",
  );
  if ($matches) {
    return shape('valid' => true, 'match' => true);
  }
  return shape(
    'valid' => true,
    'match' => false,
    'skip_reason' => 'skip-runif-locale',
  );
}

function runif_should_skip_test(
  darray<string, mixed> $options,
  string $test,
): RunifResult {
  $runif_path = find_test_ext($test, 'runif');
  if (!$runif_path) return shape('valid' => true, 'match' => true);

  $file_empty = true;
  $contents = file($runif_path, FILE_IGNORE_NEW_LINES);
  foreach ($contents as $line) {
    $line = preg_replace('/[#].*$/', '', $line); // remove comment
    $line = trim($line);
    if ($line === '') continue;
    $file_empty = false;

    $words = preg_split('/ +/', $line);
    if (count($words) < 2) {
      return shape('valid' => false, 'error' => "malformed line '$line'");
    }
    foreach ($words as $word) {
      if (!preg_match('|^[\w/.-]+$|', $word)) {
        return shape(
          'valid' => false,
          'error' => "bad word '$word' in line '$line'",
        );
      }
    }

    $type = array_shift(inout $words);
    $words = varray($words); // array_shift always promotes to darray :-\
    switch ($type) {
      case 'os':
        $result = runif_os_matches($words);
        break;
      case 'file':
        $result = runif_file_matches($words);
        break;
      case 'euid':
        $result = runif_euid_matches($options, $test, $words);
        break;
      case 'extension':
        $result = runif_extension_matches($options, $test, $words);
        break;
      case 'function':
        $result = runif_function_matches($options, $test, $words);
        break;
      case 'class':
        $result = runif_class_matches($options, $test, $words);
        break;
      case 'method':
        $result = runif_method_matches($options, $test, $words);
        break;
      case 'const':
        $result = runif_const_matches($options, $test, $words);
        break;
      case 'locale':
        $result = runif_locale_matches($options, $test, $words);
        break;
      default:
        return shape('valid' => false, 'error' => "bad match type '$type'");
    }
    if (!$result['valid'] || !$result['match']) return $result;
  }
  if ($file_empty) return shape('valid' => false, 'error' => 'empty runif file');
  return shape('valid' => true, 'match' => true);
}

function should_skip_test_simple(
  darray<string, mixed> $options,
  string $test,
): ?string {
  if ((isset($options['cli-server']) || isset($options['server'])) &&
      !can_run_server_test($test, $options)) {
    return 'skip-server';
  }

  if (isset($options['hhas-round-trip']) && substr($test, -5) === ".hhas") {
    return 'skip-hhas';
  }

  if (isset($options['hhbbc2']) || isset($options['hhas-round-trip'])) {
    $no_hhas_tag = 'nodumphhas';
    if (file_exists("$test.$no_hhas_tag") ||
        file_exists(dirname($test).'/'.$no_hhas_tag)) {
      return 'skip-nodumphhas';
    }
    if (file_exists($test . ".verify")) {
      return 'skip-verify';
    }
  }

  if (has_multi_request_mode($options) || isset($options['repo']) ||
      isset($options['server'])) {
    if (file_exists($test . ".verify")) {
      return 'skip-verify';
    }
    $no_multireq_tag = "nomultireq";
    if (file_exists("$test.$no_multireq_tag") ||
        file_exists(dirname($test).'/'.$no_multireq_tag)) {
      return 'skip-multi-req';
    }
    if (find_debug_config($test, 'hphpd.ini')) {
      return 'skip-debugger';
    }
  }

  $no_bespoke_tag = "nobespoke";
  if (isset($options['bespoke']) &&
      file_exists("$test.$no_bespoke_tag")) {
      // Skip due to changes in array identity
      return 'skip-bespoke';
  }

  $no_lazyclass_tag = "nolazyclass";
  if (isset($options['lazyclass']) &&
      file_exists("$test.$no_lazyclass_tag")) {
    return 'skip-lazyclass';
  }

  $no_jitserialize_tag = "nojitserialize";
  if (isset($options['jit-serialize']) &&
      file_exists("$test.$no_jitserialize_tag")) {
    return 'skip-jit-serialize';
  }

  return null;
}

function skipif_should_skip_test(
  darray<string, mixed> $options,
  string $test,
): RunifResult {
  $skipif_test = find_test_ext($test, 'skipif');
  if (!$skipif_test) {
    return shape('valid' => true, 'match' => true);
  }

  // Run the .skipif in non-repo mode since building a repo for it is
  // inconvenient and the same features should be available. Pick the mode
  // arbitrarily for the same reason.
  $options_without_repo = $options;
  unset($options_without_repo['repo']);
  list($hhvm, $_) = hhvm_cmd($options_without_repo, $test, $skipif_test);
  $hhvm = $hhvm[0];
  // Remove any --count <n> from the command
  $hhvm = preg_replace('/ --count[ =]\d+/', '', $hhvm);

  $descriptorspec = darray[
    0 => varray["pipe", "r"],
    1 => varray["pipe", "w"],
    2 => varray["pipe", "w"],
  ];
  $pipes = null;
  $process = proc_open("$hhvm $test 2>&1", $descriptorspec, inout $pipes);
  if (!is_resource($process)) {
    return shape(
      'valid' => false,
      'error' => 'proc_open failed while running skipif'
    );
  }

  fclose($pipes[0]);
  $output = trim(stream_get_contents($pipes[1]));
  fclose($pipes[1]);
  proc_close($process);

  // valid output is empty or a single line starting with 'skip'
  // everything else must result in a test failure
  if ($output === '') {
    return shape('valid' => true, 'match' => true);
  }
  if (preg_match('/^skip.*$/', $output)) {
    return shape(
      'valid' => true,
      'match' => false,
      'skip_reason' => 'skip-skipif',
    );
  }
  return shape('valid' => false, 'error' => "invalid skipif output '$output'");
}

function comp_line($l1, $l2, $is_reg) {
  if ($is_reg) {
    return preg_match('/^'. $l1 . '$/s', $l2);
  } else {
    return !strcmp($l1, $l2);
  }
}

function count_array_diff($ar1, $ar2, $is_reg, $idx1, $idx2, $cnt1, $cnt2,
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
      $eq = @count_array_diff($ar1, $ar2, $is_reg, $ofs1, $idx2, $cnt1,
                              $cnt2, $st);

      if ($eq > $eq1) {
        $eq1 = $eq;
      }
    }

    $eq2 = 0;
    $st = $steps;

    for ($ofs2 = $idx2 + 1; $ofs2 < $cnt2 && $st-- > 0; $ofs2++) {
      $eq = @count_array_diff($ar1, $ar2, $is_reg, $idx1, $ofs2, $cnt1, $cnt2, $st);
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
  $idx1 = 0; $cnt1 = @count($ar1);
  $idx2 = 0; $cnt2 = @count($ar2);
  $old1 = darray[];
  $old2 = darray[];

  while ($idx1 < $cnt1 && $idx2 < $cnt2) {
    if (comp_line($ar1[$idx1], $ar2[$idx2], $is_reg)) {
      $idx1++;
      $idx2++;
      continue;
    } else {
      $c1 = @count_array_diff($ar1, $ar2, $is_reg, $idx1+1, $idx2, $cnt1,
                              $cnt2, 10);
      $c2 = @count_array_diff($ar1, $ar2, $is_reg, $idx1, $idx2+1, $cnt1,
                              $cnt2, 10);

      if ($c1 > $c2) {
        $old1[$idx1] = sprintf("%03d- ", $idx1+1) . $w[$idx1++];
      } else if ($c2 > 0) {
        $old2[$idx2] = sprintf("%03d+ ", $idx2+1) . $ar2[$idx2++];
      } else {
        $old1[$idx1] = sprintf("%03d- ", $idx1+1) . $w[$idx1++];
        $old2[$idx2] = sprintf("%03d+ ", $idx2+1) . $ar2[$idx2++];
      }
    }
  }

  $diff = varray[];
  $old1_keys = array_keys($old1);
  $old2_keys = array_keys($old2);
  $old1_values = array_values($old1);
  $old2_values = array_values($old2);
  // these start at -2 so $l1 + 1 and $l2 + 1 are not valid indices
  $l1 = -2;
  $l2 = -2;
  $iter1 = 0; $end1 = count($old1);
  $iter2 = 0; $end2 = count($old2);

  while ($iter1 < $end1 || $iter2 < $end2) {
    $k1 = $iter1 < $end1 ? $old1_keys[$iter1] : null;
    $k2 = $iter2 < $end2 ? $old2_keys[$iter2] : null;
    if ($k1 == $l1 + 1 || $k2 === null) {
      $l1 = $k1;
      $diff[] = $old1_values[$iter1++];
    } else if ($k2 == $l2 + 1 || $k1 === null) {
      $l2 = $k2;
      $diff[] = $old2_values[$iter2++];
    } else if ($k1 < $k2) {
      $l1 = $k1;
      $diff[] = $old1_values[$iter1++];
    } else {
      $l2 = $k2;
      $diff[] = $old2_values[$iter2++];
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
  $m = null;
  $w = explode("\n", $wanted);
  $o = explode("\n", $output);
  if (is_null($wanted_re)) {
    $r = $w;
  } else {
    if (preg_match_with_matches('/^\((.*)\)\{(\d+)\}$/s', $wanted_re, inout $m)) {
      $t = explode("\n", $m[1]);
      $r = varray[];
      $w2 = varray[];
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

function dump_hhas_cmd(string $hhvm_cmd, $test, $hhas_file) {
  $dump_flags = implode(' ', varray[
    '-vEval.AllowHhas=true',
    '-vEval.DumpHhas=1',
    '-vEval.DumpHhasToFile='.escapeshellarg($hhas_file),
    '-vEval.LoadFilepathFromUnitCache=0',
  ]);
  $cmd = str_replace(' -- ', " $dump_flags -- ", $hhvm_cmd);
  if ($cmd == $hhvm_cmd) $cmd .= " $dump_flags";
  return $cmd;
}

function dump_hhas_to_temp(string $hhvm_cmd, $test) {
  $temp_file = Status::getTestTmpPath($test, 'round_trip.hhas');
  $cmd = dump_hhas_cmd($hhvm_cmd, $test, $temp_file);
  $ret = -1;
  system("$cmd &> /dev/null", inout $ret);
  return $ret === 0 ? $temp_file : false;
}

const SERVER_EXCLUDE_PATHS = vec[
  'quick/xenon/',
  'slow/streams/',
  'slow/ext_mongo/',
  'slow/ext_oauth/',
  'slow/ext_vsdebug/',
  'zend/good/ext/standard/tests/array/',
];
const HHAS_EXT = '.hhas';
function can_run_server_test($test, $options) {
  // explicitly disabled
  if (is_file("$test.noserver") ||
      (is_file("$test.nowebserver") && isset($options['server']))) {
    return false;
  }

  // has its own config
  if (find_test_ext($test, 'opts') || is_file("$test.ini") ||
      is_file("$test.use.for.ini.migration.testing.only.hdf")) {
    return false;
  }

  // we can't run repo only tests in server modes
  if (is_file("$test.onlyrepo") || is_file("$test.onlyjumpstart")) {
    return false;
  }

  foreach (SERVER_EXCLUDE_PATHS as $path) {
    if (strpos($test, $path) !== false) return false;
  }

  // don't run hhas tests in server modes
  if (strrpos($test, HHAS_EXT) === (strlen($test) - strlen(HHAS_EXT))) {
    return false;
  }

  return true;
}

const SERVER_TIMEOUT = 45;
function run_config_server($options, $test) {
  invariant(
    can_run_server_test($test, $options),
    'should_skip_test_simple should have skipped this',
  );

  Status::createTestTmpDir($test); // force it to be created
  $config = find_file_for_dir(dirname($test), 'config.ini');
  $port = $options['servers']['configs'][$config]->server['port'];
  $ch = curl_init("localhost:$port/$test");
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  curl_setopt($ch, CURLOPT_TIMEOUT, SERVER_TIMEOUT);
  curl_setopt($ch, CURLOPT_BINARYTRANSFER, true);
  $output = curl_exec($ch);
  if ($output is string) {
    $output = trim($output);
  } else {
    $output = "Error talking to server: " . curl_error($ch);
  }
  curl_close($ch);

  return run_config_post(varray[$output, ''], $test, $options);
}

function run_config_cli(
  $options,
  $test,
  string $cmd,
  darray<string, mixed> $cmd_env,
) {
  $cmd = timeout_prefix() . $cmd;

  if (isset($options['repo']) && !isset($options['repo-out'])) {
    // we already created it in run_test
    $cmd_env['HPHP_TEST_TMPDIR'] = Status::getTestTmpPath($test, 'tmpdir');
  } else {
    $cmd_env['HPHP_TEST_TMPDIR'] = Status::createTestTmpDir($test);
  }
  $cmd_env['HPHP_TEST_SOURCE_FILE'] = $test;
  if (isset($options['log'])) {
    $cmd_env['TRACE'] = 'printir:1';
    $cmd_env['HPHP_TRACE_FILE'] = $test . '.log';
  }

  $descriptorspec = darray[
    0 => varray["pipe", "r"],
    1 => varray["pipe", "w"],
    2 => varray["pipe", "w"],
  ];
  $pipes = null;
  $process = proc_open(
    "$cmd 2>&1", $descriptorspec, inout $pipes, null, $cmd_env
  );
  if (!is_resource($process)) {
    Status::writeDiff($test, "Couldn't invoke $cmd");
    return false;
  }

  fclose($pipes[0]);
  $output = stream_get_contents($pipes[1]);
  $output = trim($output);
  $stderr = stream_get_contents($pipes[2]);
  fclose($pipes[1]);
  fclose($pipes[2]);
  proc_close($process);

  return varray[$output, $stderr];
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
  file_put_contents(Status::getTestOutputPath($test, 'out'), $output);

  $check_hhbbc_error = isset($options['repo'])
    && (file_exists($test . '.hhbbc_assert') ||
        file_exists($test . '.hphpc_assert'));

  // hhvm redirects errors to stdout, so anything on stderr is really bad.
  if ($stderr && !$check_hhbbc_error) {
    Status::writeDiff(
      $test,
      "Test failed because the process wrote on stderr:\n$stderr"
    );
    return false;
  }

  $repeats = 0;
  if (!$check_hhbbc_error) {
    if (isset($options['retranslate-all'])) {
      $repeats = (int)$options['retranslate-all'] * 2;
    }

    if (isset($options['recycle-tc'])) {
      $repeats = (int)$options['recycle-tc'];
    }

    if (isset($options['cli-server'])) {
      $repeats = 3;
    }
  }

  list($file, $type) = get_expect_file_and_type($test, $options);
  if ($file === null || $type === null) {
    Status::writeDiff(
      $test,
      "No $test.expect, $test.expectf, $test.hhvm.expect, " .
      "$test.hhvm.expectf, or $test.expectregex. " .
      "If $test is meant to be included by other tests, " .
      "use a different file extension.\n"
    );
    return false;
  }

  if ($type === 'expect' || $type === 'hhvm.expect') {
    $wanted = trim(file_get_contents($file));
    if (isset($options['ignore-oids']) || isset($options['repo'])) {
      $output = replace_object_resource_ids($output, 'n');
      $wanted = replace_object_resource_ids($wanted, 'n');
    }

    if (!$repeats) {
      $passed = !strcmp($output, $wanted);
      if (!$passed) {
        Status::writeDiff($test, generate_diff($wanted, null, $output));
      }
      return $passed;
    }
    $wanted_re = preg_quote($wanted, '/');
  } else if ($type === 'expectf' || $type === 'hhvm.expectf') {
    $wanted = trim(file_get_contents($file));
    if (isset($options['ignore-oids']) || isset($options['repo'])) {
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
      varray['%binary_string_optional%'],
      'string',
      $wanted_re
    );
    $wanted_re = str_replace(
      varray['%unicode_string_optional%'],
      'string',
      $wanted_re
    );
    $wanted_re = str_replace(
      varray['%unicode\|string%', '%string\|unicode%'],
      'string',
      $wanted_re
    );
    $wanted_re = str_replace(
      varray['%u\|b%', '%b\|u%'],
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
  } else if ($type === 'expectregex') {
    $wanted_re = trim(file_get_contents($file));
  } else {
    throw new Exception("Unsupported expect file type: ".$type);
  }

  if ($repeats) {
    $wanted_re = "($wanted_re\s*)".'{'.$repeats.'}';
  }
  if (!isset($wanted)) $wanted = $wanted_re;
  $passed = @preg_match("/^$wanted_re\$/s", $output);
  if ($passed) return true;
  if ($passed === false && $repeats) {
    // $repeats can cause the regex to become too big, and fail
    // to compile.
    return 'skip-repeats-fail';
  }
  $diff = generate_diff($wanted_re, $wanted_re, $output);
  if ($passed === false && $diff === "") {
    // the preg match failed, probably because the regex was too complex,
    // but since the line by line diff came up empty, we're fine
    return true;
  }
  Status::writeDiff($test, $diff);
  return false;
}

function timeout_prefix() {
  if (is_executable('/usr/bin/timeout')) {
    return '/usr/bin/timeout ' . TIMEOUT_SECONDS . ' ';
  } else {
    return hphp_home() . '/hphp/tools/timeout.sh -t ' . TIMEOUT_SECONDS . ' ';
  }
}

function run_foreach_config(
  $options,
  $test,
  varray<string> $cmds,
  darray<string, mixed> $cmd_env,
) {
  invariant(count($cmds) > 0, "run_foreach_config: no modes");
  foreach ($cmds as $cmd) {
    $outputs = run_config_cli($options, $test, $cmd, $cmd_env);
    if ($outputs === false) return false;
    $result = run_config_post($outputs, $test, $options);
    if (!$result) return $result;
  }
  return $result;
}

function run_and_log_test($options, $test) {
  $stime = time();
  $time = microtime(true);
  $status = run_test($options, $test);
  $time = microtime(true) - $time;
  $etime = time();

  if ($status === false) {
    $diff = Status::diffForTest($test);
    if ($diff === '') {
      $diff = 'Test failed with empty diff';
    }
    Status::fail($test, $time, $stime, $etime, $diff);
  } else if ($status === true) {
    Status::pass($test, $time, $stime, $etime);
    clean_intermediate_files($test, $options);
  } else if ($status is string) {
    invariant(
      preg_match('/^skip-[\w-]+$/', $status),
      "invalid skip status $status"
    );
    Status::skip($test, substr($status, 5), $time, $stime, $etime);
    clean_intermediate_files($test, $options);
  } else {
    invariant_violation("invalid status type " . gettype($status));
  }
}

function run_test($options, $test) {
  $skip_reason = should_skip_test_simple($options, $test);
  if ($skip_reason !== null) return $skip_reason;

  if (!($options['no-skipif'] ?? false)) {
    $result = runif_should_skip_test($options, $test);
    if (!$result['valid']) {
      invariant($result['error'] is string, 'missing runif error');
      Status::writeDiff($test, 'Invalid .runif file: ' . $result['error']);
      return false;
    }
    if (!$result['match']) {
      invariant($result['skip_reason'] is string, 'missing skip_reason');
      return $result['skip_reason'];
    }

    $result = skipif_should_skip_test($options, $test);
    if (!$result['valid']) {
      invariant($result['error'] is string, 'missing skipif error');
      Status::writeDiff($test, $result['error']);
      return false;
    }
    if (!$result['match']) {
      invariant($result['skip_reason'] is string, 'missing skip_reason');
      return $result['skip_reason'];
    }
  }

  list($hhvm, $hhvm_env) = hhvm_cmd($options, $test);

  if (preg_grep('/ --count[ =][0-9]+ .* --count[ =][0-9]+( |$)/', $hhvm)) {
    // we got --count from 2 sources (e.g. .opts file and multi_request_mode)
    // this can't work so skip the test
    return 'skip-count';
  } else if (isset($options['jit-serialize'])) {
    // jit-serialize adds the --count option later, so even 1 --count in the
    // command means we have to skip
    if (preg_grep('/ --count[ =][0-9]+( |$)/', $hhvm)) {
      return 'skip-count';
    }
  }

  if (isset($options['repo'])) {
    if (file_exists($test.'.norepo')) {
      return 'skip-norepo';
    }
    if (file_exists($test.'.onlyjumpstart') &&
       (!isset($options['jit-serialize']) || (int)$options['jit-serialize'] < 1)) {
      return 'skip-onlyjumpstart';
    }

    $test_repo = test_repo($options, $test);
    if (isset($options['repo-out'])) {
      // we may need to clean up after a previous run
      $repo_files = vec['hhvm.hhbc', 'hhvm.hhbbc', 'hackc.hhbc', 'hackc.hhbbc'];
      foreach ($repo_files as $repo_file) {
        @unlink("$test_repo/$repo_file");
      }
    } else {
      // create tmpdir now so that we can write repos
      Status::createTestTmpDir($test);
    }

    $program = isset($options['hackc']) ? "hackc" : "hhvm";

    if (file_exists($test . '.hphpc_assert')) {
      $hphp = hphp_cmd($options, $test, $program);
      return run_foreach_config($options, $test, varray[$hphp], $hhvm_env);
    } else if (file_exists($test . '.hhbbc_assert')) {
      $hphp = hphp_cmd($options, $test, $program);
      if (repo_separate($options, $test)) {
        $result = exec_with_stack($hphp);
        if ($result !== true) return false;
        $hhbbc = hhbbc_cmd($options, $test, $program);
        return run_foreach_config($options, $test, varray[$hhbbc], $hhvm_env);
      } else {
        return run_foreach_config($options, $test, varray[$hphp], $hhvm_env);
      }
    }

    if (!repo_mode_compile($options, $test, $program)) {
      return false;
    }

    if (isset($options['hhbbc2'])) {
      invariant(
        count($hhvm) === 1,
        "get_options forbids modes because we're not runnig code"
      );
      // create tmpdir now so that we can write hhas
      Status::createTestTmpDir($test);
      $hhas_temp1 = dump_hhas_to_temp($hhvm[0], "$test.before");
      if ($hhas_temp1 === false) {
        Status::writeDiff($test, "dumping hhas after first hhbbc pass failed");
        return false;
      }
      shell_exec("mv $test_repo/$program.hhbbc $test_repo/$program.hhbc");
      $hhbbc = hhbbc_cmd($options, $test, $program);
      $result = exec_with_stack($hhbbc);
      if ($result !== true) {
        Status::writeDiff($test, $result);
        return false;
      }
      $hhas_temp2 = dump_hhas_to_temp($hhvm[0], "$test.after");
      if ($hhas_temp2 === false) {
        Status::writeDiff($test, "dumping hhas after second hhbbc pass failed");
        return false;
      }
      $diff = shell_exec("diff $hhas_temp1 $hhas_temp2");
      if (trim($diff) !== '') {
        Status::writeDiff($test, $diff);
        return false;
      }
    }

    if (isset($options['jit-serialize'])) {
      invariant(count($hhvm) === 1, 'get_options enforces jit mode only');
      $cmd = jit_serialize_option($hhvm[0], $test, $options, true);
      $outputs = run_config_cli($options, $test, $cmd, $hhvm_env);
      if ($outputs === false) return false;
      $cmd = jit_serialize_option($hhvm[0], $test, $options, true);
      $outputs = run_config_cli($options, $test, $cmd, $hhvm_env);
      if ($outputs === false) return false;
      $hhvm[0] = jit_serialize_option($hhvm[0], $test, $options, false);
    }

    return run_foreach_config($options, $test, $hhvm, $hhvm_env);
  }

  if (file_exists($test.'.onlyrepo')) {
    return 'skip-onlyrepo';
  }
  if (file_exists($test.'.onlyjumpstart')) {
    return 'skip-onlyjumpstart';
  }

  if (isset($options['hhas-round-trip'])) {
    invariant(
      substr($test, -5) !== ".hhas",
      'should_skip_test_simple should have skipped this',
    );
    // create tmpdir now so that we can write hhas
    Status::createTestTmpDir($test);
    // dumping hhas, not running code so arbitrarily picking a mode
    $hhas_temp = dump_hhas_to_temp($hhvm[0], $test);
    if ($hhas_temp === false) {
      $err = "system failed: " .
        dump_hhas_cmd($hhvm[0], $test,
          Status::getTestTmpPath($test, 'round_trip.hhas')) .
        "\n";
      Status::writeDiff($test, $err);
      return false;
    }
    list($hhvm, $hhvm_env) = hhvm_cmd($options, $test, $hhas_temp);
  }

  if (isset($options['server'])) {
    return run_config_server($options, $test);
  }
  return run_foreach_config($options, $test, $hhvm, $hhvm_env);
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
      $output = null;
      $return_var = -1;
      return exec('sysctl -n hw.ncpu', inout $output, inout $return_var);
  }
  return 2; // default when we don't know how to detect.
}

function make_header($str) {
  return "\n\033[0;33m".$str."\033[0m\n";
}

function print_commands($tests, $options) {
  if (isset($options['verbose'])) {
    print make_header("Run these by hand:");
  } else {
    $test = $tests[0];
    print make_header("Run $test by hand:");
    $tests = varray[$test];
  }

  foreach ($tests as $test) {
    list($commands, $_) = hhvm_cmd($options, $test);
    if (!isset($options['repo'])) {
      foreach ($commands as $c) {
        print "$c\n";
      }
      continue;
    }

    // How to run it with hhbbc:
    $program = isset($options['hackc']) ? "hackc" : "hhvm";
    $hhbbc_cmds = hphp_cmd($options, $test, $program)."\n";
    if (repo_separate($options, $test)) {
      $hhbbc_cmd  = hhbbc_cmd($options, $test, $program)."\n";
      $hhbbc_cmds .= $hhbbc_cmd;
      if (isset($options['hhbbc2'])) {
        foreach ($commands as $c) {
          $hhbbc_cmds .=
            $c." -vEval.DumpHhas=1 > $test.before.round_trip.hhas\n";
        }
        $hhbbc_cmds .=
          "mv $test_repo/$program.hhbbc $test_repo/$program.hhbc\n";
        $hhbbc_cmds .= $hhbbc_cmd;
        foreach ($commands as $c) {
          $hhbbc_cmds .=
            $c." -vEval.DumpHhas=1 > $test.after.round_trip.hhas\n";
        }
        $hhbbc_cmds .=
          "diff $test.before.round_trip.hhas $test.after.round_trip.hhas\n";
      }
    }
    if (isset($options['jit-serialize'])) {
      invariant(count($commands) === 1, 'get_options enforces jit mode only');
      $hhbbc_cmds .=
        jit_serialize_option($commands[0], $test, $options, true) . "\n";
      $hhbbc_cmds .=
        jit_serialize_option($commands[0], $test, $options, true) . "\n";
      $commands[0] = jit_serialize_option($commands[0], $test, $options, false);
    }
    foreach ($commands as $c) {
      $hhbbc_cmds .= $c."\n";
    }
    print "$hhbbc_cmds\n";
  }
}

// This runs only in the "printer" child.
function msg_loop($num_tests, $queue) {
  $do_progress =
    (
      Status::getMode() === Status::MODE_NORMAL ||
      Status::getMode() === Status::MODE_RECORD_FAILURES
    ) &&
    Status::hasCursorControl();

  if ($do_progress) {
    $stty = strtolower(Status::getSTTY());
    $matches = null;
    if (preg_match_with_matches("/columns ([0-9]+);/", $stty, inout $matches) ||
        // because BSD has to be different
        preg_match_with_matches("/([0-9]+) columns;/", $stty, inout $matches)) {
      $cols = $matches[1];
    } else {
      $do_progress = false;
    }
  }

  while (true) {
    list($pid, $type, $message) = $queue->receiveMessage();
    if (!Status::handle_message($type, $message)) break;

    if ($do_progress) {
      $total_run = (Status::$skipped + Status::$failed + Status::$passed);
      $bar_cols = ((int)$cols - 45);

      $passed_ticks  = round($bar_cols * (Status::$passed  / $num_tests));
      $skipped_ticks = round($bar_cols * (Status::$skipped / $num_tests));
      $failed_ticks  = round($bar_cols * (Status::$failed  / $num_tests));

      $fill = $bar_cols - ($passed_ticks + $skipped_ticks + $failed_ticks);
      if ($fill < 0) $fill = 0;

      $fill = str_repeat('-', (int)$fill);

      $passed_ticks = str_repeat('#',  (int)$passed_ticks);
      $skipped_ticks = str_repeat('#', (int)$skipped_ticks);
      $failed_ticks = str_repeat('#',  (int)$failed_ticks);

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
      $reasons = Status::$skip_reasons;
      arsort(inout $reasons);
      Status::$skip_reasons = $reasons;
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
    if (!($options['no-fun'] ?? false)) {
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
    if (!($options['no-fun'] ?? false)) {
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
  if (!($options['no-fun'] ?? false)) {
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
  if ($options['failure-file'] ?? false) {
    @unlink($options['failure-file']);
  }
  if (isset($options['verbose'])) {
    print_commands($tests, $options);
  }
}

function print_failure($argv, $results, $options) {
  $failed = varray[];
  $passed = varray[];
  foreach ($results as $result) {
    if ($result['status'] === 'failed') {
      $failed[] = $result['name'];
    }
    if ($result['status'] === 'passed') {
      $passed[] = $result['name'];
    }
  }
  sort(inout $failed);

  $failing_tests_file = ($options['failure-file'] ?? false)
    ? $options['failure-file']
    : Status::getRunTmpDir() . '/test-failures';
  file_put_contents($failing_tests_file, implode("\n", $failed)."\n");
  if ($passed) {
    $passing_tests_file = ($options['success-file'] ?? false)
      ? $options['success-file']
      : Status::getRunTmpDir() . '/tests-passed';
    file_put_contents($passing_tests_file, implode("\n", $passed)."\n");
  }

  print "\n".count($failed)." tests failed\n";
  if (!($options['no-fun'] ?? false)) {
    // Unicode for table-flipping emoticon
    // https://knowyourmeme.com/memes/flipping-tables
    print "(\u{256F}\u{00B0}\u{25A1}\u{00B0}\u{FF09}\u{256F}\u{FE35} \u{253B}";
    print "\u{2501}\u{253B}\n";
  }

  print_commands($failed, $options);

  print make_header("See failed test output and expectations:");
  foreach ($failed as $n => $test) {
    if ($n !== 0) print "\n";
    print 'cat ' . Status::getTestOutputPath($test, 'diff') . "\n";
    print 'cat ' . Status::getTestOutputPath($test, 'out') . "\n";
    $expect_file = get_expect_file_and_type($test, $options)[0];
    if ($expect_file is null) {
      print "# no expect file found for $test\n";
    } else {
      print "cat $expect_file\n";
    }

    // only print 3 tests worth unless verbose is on
    if ($n === 2 && !isset($options['verbose'])) {
      $remaining = count($failed) - 1 - $n;
      if ($remaining > 0) {
        print make_header("... and $remaining more.");
      }
      break;
    }
  }

  if ($passed) {
    print make_header(
      'For xargs, lists of failed and passed tests are available using:'
    );
    print 'cat '.$failing_tests_file."\n";
    print 'cat '.$passing_tests_file."\n";
  } else {
    print make_header('For xargs, list of failures is available using:').
      'cat '.$failing_tests_file."\n";
  }

  print
    make_header("Re-run just the failing tests:") .
    str_replace("run.php", "run", $argv[0]) . ' ' .
    implode(' ', \HH\global_get('recorded_options')) .
    sprintf(' $(cat %s)%s', $failing_tests_file, "\n");
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
  $threads = get_num_threads($options, PHP_INT_MAX);
  $thread_option = isset($options['cli-server'])
    ? '-vEval.UnixServerWorkers='.$threads
    : '-vServer.ThreadCount='.$threads;
  $prelude = isset($options['server'])
    ? '-vEval.PreludePath=' . Status::getRunTmpDir() . '/server-prelude.php'
    : "";
  $command = hhvm_cmd_impl(
    $options,
    $config,
    null, // we do not pass Autoload.DB.Path to the server process
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
    $prelude,

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
  if (count($command) !== 1) {
    error("Can't run multi-mode tests in server mode");
  }
  $command = $command[0];
  if (getenv('HHVM_TEST_SERVER_LOG')) {
    echo "Starting server '$command'\n";
  }

  $descriptors = darray[
    0 => varray['file', '/dev/null', 'r'],
    1 => varray['file', '/dev/null', 'w'],
    2 => varray['file', '/dev/null', 'w'],
  ];

  $dummy = null;
  $proc = proc_open($command, $descriptors, inout $dummy);
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

final class ServerRef {
  public function __construct(public $server) {
  }
}

/*
 * For each config file in $configs, start up a server on a randomly-determined
 * port. Return value is an array mapping pids and config files to arrays of
 * information about the server.
 */
function start_servers($options, $configs) {
  if (isset($options['server'])) {
    $prelude = <<<'EOT'
<?hh
<<__EntryPoint>> function UNIQUE_NAME_I_DONT_EXIST_IN_ANY_TEST(): void {
  putenv("HPHP_TEST_TMPDIR=BASEDIR{$_SERVER['SCRIPT_NAME']}.tmpdir");
}
EOT;
    file_put_contents(
      Status::getRunTmpDir() . '/server-prelude.php',
      str_replace('BASEDIR', Status::getRunTmpDir(), $prelude),
    );
  }

  $starting = varray[];
  foreach ($configs as $config) {
    $starting[] = start_server_proc($options, $config, find_open_port());
  }

  $start_time = microtime(true);
  $servers = darray['pids' => darray[], 'configs' => darray[]];

  // Wait for all servers to come up.
  while (count($starting) > 0) {
    $still_starting = varray[];

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
        $ref = new ServerRef($server);
        $servers['pids'][$server['pid']] = $ref;
        $servers['configs'][$server['config']] = $ref;
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

function get_num_threads($options, $tests) {
  if (isset($options['threads'])) {
    $threads = (int)$options['threads'];
    if ((string)$threads !== $options['threads'] || $threads < 1) {
      error("--threads must be an integer >= 1");
    }
  } else {
    $threads = isset($options['server']) || isset($options['cli-server'])
      ? num_cpus() * 2 : num_cpus();
  }
  return min(count($tests), $threads);
}

function runner_precheck() {
  // basic checking for runner.
  if (!((bool)$_SERVER ?? false) || !((bool)$_ENV ?? false)) {
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
    success(list_tests($files, $options));
  }

  $tests = find_tests($files, $options);
  if (isset($options['shuffle'])) {
    shuffle($tests);
  }

  // Explicit path given by --hhvm-binary-path takes priority. Then, if an
  // HHVM_BIN env var exists, and the file it points to exists, that trumps
  // any default hhvm executable path.
  if (isset($options['hhvm-binary-path'])) {
    $binary_path = check_executable($options['hhvm-binary-path']);
    putenv("HHVM_BIN=" . $binary_path);
  } else if (getenv("HHVM_BIN") !== false) {
    $binary_path = check_executable(getenv("HHVM_BIN"));
  } else {
    check_for_multiple_default_binaries();
    $binary_path = hhvm_path();
  }

  if (isset($options['verbose'])) {
    print "You are using the binary located at: " . $binary_path . "\n";
  }

  Status::createTmpDir();

  $servers = null;
  if (isset($options['server']) || isset($options['cli-server'])) {
    if (isset($options['server']) && isset($options['cli-server'])) {
      error("Server mode and CLI Server mode are mutually exclusive");
    }
    if (isset($options['repo'])) {
      error("Server mode repo tests are not supported");
    }

    /* We need to start up a separate server process for each config file
     * found. */
    $configs = keyset[];
    foreach ($tests as $test) {
      $config = find_file_for_dir(dirname($test), 'config.ini');
      if (!$config) {
        error("Couldn't find config file for $test");
      }
      if (array_key_exists($config, $configs)) continue;
      if (should_skip_test_simple($options, $test) !== null) continue;
      $configs[] = $config;
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
  // Get the serial tests to be in their own bucket later.
  $serial_tests = serial_only_tests($tests);

  // If we have no serial tests, we can use the maximum number of allowed
  // threads for the test running. If we have some, we save one thread for
  // the serial bucket. However if we only have one thread, we don't split
  // out serial tests.
  $parallel_threads = get_num_threads($options, $tests);
  if ($parallel_threads === 1) {
    $test_buckets = varray[$tests];
  } else {
    if (count($serial_tests) > 0) {
      // reserve a thread for serial tests
      $parallel_threads--;
    }

    $test_buckets = varray[];
    for ($i = 0; $i < $parallel_threads; $i++) {
      $test_buckets[] = varray[];
    }

    $i = 0;
    foreach ($tests as $test) {
      if (!in_array($test, $serial_tests)) {
        $test_buckets[$i][] = $test;
        $i = ($i + 1) % $parallel_threads;
      }
    }

    if (count($serial_tests) > 0) {
      // The last bucket is serial.
      $test_buckets[] = $serial_tests;
    }
  }

  // Remember that the serial tests are also in the tests array too,
  // so they are part of the total count.
  if (!isset($options['testpilot'])) {
    print "Running ".count($tests)." tests in ".
      count($test_buckets)." threads (" . count($serial_tests) .
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

  Status::$nofork = count($tests) == 1 && !$servers;

  if (!Status::$nofork) {
    // Create the Queue before any children are forked.
    $queue = Status::getQueue();

    // Fork a "printer" child to process status messages.
    $printer_pid = pcntl_fork();
    if ($printer_pid == -1) {
      error("failed to fork");
    } else if ($printer_pid == 0) {
      msg_loop(count($tests), $queue);
      return 0;
    }
  }

  // NOTE: This unblocks the Queue (if needed).
  Status::started();

  // Fork "worker" children (if needed).
  $children = darray[];
  // We write results as json in each child and collate them at the end
  $json_results_files = varray[];
  if (Status::$nofork) {
    Status::registerCleanup(isset($options['no-clean']));
    $json_results_file = tempnam('/tmp', 'test-run-');
    $json_results_files[] = $json_results_file;
    invariant(count($test_buckets) === 1, "nofork was set erroneously");
    $return_value = child_main($options, $test_buckets[0], $json_results_file);
  } else {
    foreach ($test_buckets as $test_bucket) {
      $json_results_file = tempnam('/tmp', 'test-run-');
      $json_results_files[] = $json_results_file;
      $pid = pcntl_fork();
      if ($pid == -1) {
        error('could not fork');
      } else if ($pid) {
        $children[$pid] = $pid;
      } else {
        exit(child_main($options, $test_bucket, $json_results_file));
      }
    }

    // Make sure to clean up on exit, or on SIGTERM/SIGINT.
    // Do this here so no children inherit this.
    Status::registerCleanup(isset($options['no-clean']));

    // Have the parent wait for all forked children to exit.
    $return_value = 0;
    while (count($children) && $printer_pid != 0) {
      $status = null;
      $pid = pcntl_wait(inout $status);
      if (!pcntl_wifexited($status) && !pcntl_wifsignaled($status)) {
        error("Unexpected exit status from child");
      }

      if ($pid == $printer_pid) {
        // We should be finishing up soon.
        $printer_pid = 0;
      } else if ($servers && isset($servers['pids'][$pid])) {
        // A server crashed. Restart it.
        if (getenv('HHVM_TEST_SERVER_LOG')) {
          echo "\nServer $pid crashed. Restarting.\n";
        }
        Status::serverRestarted();
        $ref = $servers['pids'][$pid];
        $ref->server =
          start_server_proc($options, $ref->server['config'], $ref->server['port']);

        // Unset the old $pid entry and insert the new one.
        unset($servers['pids'][$pid]);
        $servers['pids'][$ref->server['pid']] = $ref;
      } elseif (isset($children[$pid])) {
        unset($children[$pid]);
        $return_value |= pcntl_wexitstatus($status);
      } // Else, ignorable signal
    }
  }

  Status::finished($return_value);

  // Wait for the printer child to die, if needed.
  if (!Status::$nofork && $printer_pid != 0) {
    $status = 0;
    $pid = pcntl_waitpid($printer_pid, inout $status);
    if (!pcntl_wifexited($status) && !pcntl_wifsignaled($status)) {
      error("Unexpected exit status from child");
    }
  }

  // Kill the servers.
  if ($servers) {
    foreach ($servers['pids'] as $ref) {
      proc_terminate($ref->server['proc']);
      proc_close($ref->server['proc']);
    }
  }

  // aggregate results
  $results = darray[];
  foreach ($json_results_files as $json_results_file) {
    $json = json_decode(file_get_contents($json_results_file), true);
    if (!is_array($json)) {
      error(
        "\nNo JSON output was received from a test thread. ".
        "Either you killed it, or it might be a bug in the test script."
      );
    }
    $results = array_merge($results, $json);
    unlink($json_results_file);
  }

  // print results
  if (isset($options['record-failures'])) {
    $fail_file = $options['record-failures'];
    $failed_tests = varray[];
    $prev_failing = varray[];
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
    sort(inout $failed_tests);
    file_put_contents($fail_file, implode("\n", $failed_tests));
  } else if (isset($options['testpilot'])) {
    Status::say(darray['op' => 'all_done', 'results' => $results]);
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
                   Status::addTestTimesSerial($results)));

  return $return_value;
}

<<__EntryPoint>>
function run_main(): void {
  exit(main(\HH\global_get('argv')));
}
