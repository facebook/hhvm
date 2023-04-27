<?hh
/**
* Run the test suites in various configurations.
*/

use namespace HH\Lib\C;

const int TIMEOUT_SECONDS = 300;

const string MULTI_REQUEST_SEP = '//<>!<>&&|MULTI_REQUEST_SEP|&&<>!<>\\';

function get_argv(): vec<string> {
  return \HH\FIXME\UNSAFE_CAST<vec<mixed>,vec<string>>(
    \HH\global_get('argv') as vec<_>
  );
}

function mtime(): float {
  return microtime(true) as float;
}

// The "HPHP_HOME" environment variable can be set (to ".../fbcode"), to
// define "hphp_home()" and (indirectly) "test_dir()".  Otherwise, we will use
// "__DIR__" as "test_dir()", and its grandparent directory for "hphp_home()"
// (unless we are testing a dso extensions).

<<__Memoize>>
function is_testing_dso_extension(): bool {
  $home = getenv("HPHP_HOME");
  if ($home is string) {
    return false;
  }
  // detecting if we're running outside of the hhvm codebase.
  return !is_file(__DIR__."/../../hphp/test/run.php");
}

<<__Memoize>>
function hphp_home(): string {
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

function get_expect_file_and_type(
  string $test,
  Options $options,
): vec<?string> {
  $types = vec[
    'expect',
    'expectf',
  ];
  if ($options->repo) {
    if (file_exists($test . '.hphpc_assert')) {
      return vec[$test . '.hphpc_assert', 'expectf'];
    }
    if (file_exists($test . '.hhbbc_assert')) {
      return vec[$test . '.hhbbc_assert', 'expectf'];
    }
    foreach ($types as $type) {
      $fname = "$test.$type-repo";
      if (file_exists($fname)) {
        return vec[$fname, $type];
      }
    }
  }

  foreach ($types as $type) {
    $fname = "$test.$type";
    if (file_exists($fname)) {
      return vec[$fname, $type];
    }
  }
  return vec[null, null];
}

function multi_request_modes(Options $options): vec<string> {
  $r = vec[];
  if ($options->retranslate_all is nonnull) $r []= 'retranslate-all';
  if ($options->recycle_tc is nonnull) $r []= 'recycle-tc';
  if ($options->jit_serialize is nonnull) $r []= 'jit-serialize';
  if ($options->cli_server) $r []= 'cli-server';
  return $r;
}

function has_multi_request_mode(Options $options): bool {
  return count(multi_request_modes($options)) != 0;
}

function test_repo(Options $options, string $test): string {
  if ($options->repo_out is nonnull) {
    return $options->repo_out . '/' . str_replace('/', '.', $test) . '.repo';
  }
  return Status::getTestTmpPath($test, 'repo');
}

function jit_serialize_option(
  string $cmd, string $test, Options $options, bool $serialize,
): string {
  $serialized = test_repo($options, $test) . "/jit.dump";
  $cmds = explode(' -- ', $cmd, 2);
  $jit_serialize = (int)($options->jit_serialize ?? 0);
  $cmds[0] .=
    ' --count=' . ($serialize ? $jit_serialize + 1 : 1) .
    " -vEval.JitSerdesFile=\"" . $serialized . "\"" .
    " -vEval.JitSerdesMode=" . ($serialize ? 'Serialize' : 'DeserializeOrFail') .
    ($serialize ? " -vEval.JitSerializeOptProfRequests=" . $jit_serialize : '');
  if ($options->jitsample is nonnull && $serialize) {
    $cmds[0] .= ' -vDeploymentId="' . $options->jitsample . '-serialize"';
  }
  return implode(' -- ', $cmds);
}

function usage(): string {
  $argv = get_argv();
  return "usage: {$argv[0]} [-m jit|interp] [-r] <test/directories>";
}

function error(string $message): noreturn {
  print "$message\n";
  exit(1);
}

// If a user-supplied path is provided, let's make sure we have a valid
// executable. Returns canonicanalized path or exits.
function check_executable(string $path): string {
  $rpath = realpath($path);
  if ($rpath === false || !is_executable($rpath)) {
    error("Provided HHVM executable ($path) is not an executable file.\n" .
          "If using HHVM_BIN, make sure that is set correctly.");
  }

  $output = vec[];
  $return_var = -1;
  exec($rpath . " --version 2> /dev/null", inout $output, inout $return_var);
  if (strpos(implode("", $output), "HipHop ") !== 0) {
    error("Provided file ($rpath) is not an HHVM executable.\n" .
          "If using HHVM_BIN, make sure that is set correctly.");
  }

  return $rpath;
}

function hhvm_binary_routes(): dict<string, string> {
  return dict[
    "buck"    => "/buck-out/gen/hphp/hhvm/hhvm",
    "buck2"   => "/../buck-out/v2/gen/fbcode/hphp/hhvm/out",
    "cmake"   => "/hphp/hhvm"
  ];
}

// For Facebook: We have several build systems, and we can use any of them in
// the same code repo.  If multiple binaries exist, we want the onus to be on
// the user to specify a particular one because before we chose the buck one
// by default and that could cause unexpected results.
function check_for_multiple_default_binaries(): void {
  // Env var we use in testing that'll pick which build system to use.
  if (getenv("FBCODE_BUILD_TOOL") !== false) {
    return;
  }

  $home = hphp_home();
  $found = vec[];
  foreach (hhvm_binary_routes() as $path) {
    $abs_path = $home . $path . "/hhvm";
    if (file_exists($abs_path)) {
      $found[] = $abs_path;
    }
  }

  // emacs hack mode thinks <= is some kind of anchor and messes up
  // parsing after it...
  if (!(count($found) > 1)) return;

  $msg = "Multiple binaries exist in this repo. \n";
  foreach ($found as $bin) {
    $msg .= " - " . $bin . "\n";
  }
  $msg .= "Are you in fbcode?  If so, remove a binary \n"
    . "or use the --hhvm-binary-path option to the test runner. \n"
    . "e.g. test/run --hhvm-binary-path /path/to/binary slow\n";
  error($msg);
}

function hhvm_path(): string {
  $file = "";
  $hhvm_bin = getenv("HHVM_BIN");
  if ($hhvm_bin is string) {
    $file = realpath($hhvm_bin);
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

function bin_root(): string {
  $hhvm_bin = getenv("HHVM_BIN");
  if ($hhvm_bin is string) {
    return dirname(realpath($hhvm_bin));
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

function unit_cache_file(): string {
  return Status::getTmpPathFile('unit-cache.sql');
}

function read_opts_file(?string $file): string {
  if ($file is null || !file_exists($file)) {
    return "";
  }
  $fp = fopen($file, "r");
  invariant($fp is resource, "%s", __METHOD__);

  $contents = "";
  for ($line = fgets($fp); $line; $line = fgets($fp)) {
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
function rel_path(string $to): string {
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
    do {
      $relPath[] = '..';
      $remaining--;
    } while ($remaining > 0);
  } else {
    $relPath[] = '.';
  }
  while ($d < $to_len) {
    $relPath[] = $to[$d];
    $d++;
  }
  return implode('/', $relPath);
}

// Keep this in sync with the dict in get_options() below.
// Options taking a value (with a trailing `:` in the dict key)
// should be ?string. Otherwise they should be bool.
final class Options {
    public ?string $env;
    public ?string $exclude;
    public ?string $exclude_pattern;
    public ?string $exclude_recorded_failures;
    public ?string $include;
    public ?string $include_pattern;
    public bool $repo = false;
    public bool $split_hphpc = false;
    public bool $repo_single = false;
    public bool $repo_separate = false;
    public bool $only_remote_executable = false;
    public bool $only_non_remote_executable = false;
    public ?string $repo_threads;
    public ?string $repo_out;
    public bool $hhbbc2 = false;
    public ?string $mode;
    public bool $server = false;
    public bool $cli_server = false;
    public bool $shuffle = false;
    public bool $help = false;
    public bool $verbose = false;
    public bool $testpilot = false;
    public ?string $threads;
    public ?string $args;
    public ?string $compiler_args;
    public bool $log = false;
    public ?string $failure_file;
    public bool $wholecfg = false;
    public bool $hhas_round_trip = false;
    public bool $verify_hackc_translator = false;
    public bool $color = false;
    public bool $no_fun = false;
    public bool $no_skipif = false;
    public bool $cores = false;
    public bool $dump_tc = false;
    public bool $no_clean = false;
    public bool $list_tests = false;
    public ?string $recycle_tc;
    public ?string $retranslate_all;
    public ?string $jit_serialize;
    public ?string $hhvm_binary_path;
    public ?string $working_dir;
    public ?string $vendor;
    public ?string $record_failures;
    public ?string $ignore_oids;
    public ?string $jitsample;
    public ?string $hh_single_type_check;
    public bool $write_to_checkout = false;
    public bool $bespoke = false;

    // Additional state added for convenience since Options is plumbed
    // around almost everywhere.
    public ?Servers $servers = null;
}

function get_options(
  vec<string> $argv,
): (Options, vec<string>) {
  // Options marked * affect test behavior, and need to be reported by list_tests.
  // Options with a trailing : take a value.
  $parameters = dict[
    '*env:' => '',
    'exclude:' => 'e:',
    'exclude-pattern:' => 'E:',
    'exclude-recorded-failures:' => 'x:',
    'include:' => 'i:',
    'include-pattern:' => 'I:',
    '*repo' => 'r',
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
    'only-remote-executable' => '',
    'only-non-remote-executable' => '',
    'threads:' => '',
    '*args:' => 'a:',
    '*compiler-args:' => '',
    'log' => 'l',
    'failure-file:' => '',
    '*wholecfg' => '',
    '*hhas-round-trip' => '',
    '*verify-hackc-translator' => '',
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
    '*working-dir:' => 'w:',
    '*vendor:' => '',
    'record-failures:' => '',
    '*ignore-oids' => '',
    'jitsample:' => '',
    '*hh_single_type_check:' => '',
    'write-to-checkout' => '',
    'bespoke' => '',
  ];
  $options = new Options() as dynamic;
  $files = vec[];
  $recorded = vec[];

  /*
   * '-' argument causes all future arguments to be treated as filenames, even
   * if they would otherwise match a valid option. Otherwise, arguments starting
   * with '-' MUST match a valid option.
   */
  $force_file = false;

  for ($i = 1; $i < count($argv); $i++) {
    $arg = $argv[$i];

    if (strlen($arg) === 0) {
      continue;
    } else if ($force_file) {
      $files[] = $arg;
    } else if ($arg === '-') {
      $forcefile = true;
    } else if ($arg[0] === '-') {
      $found = false;

      foreach ($parameters as $long => $short) {
        if ($arg == '-'.str_replace(':', '', $short) ||
            $arg == '--'.str_replace(vec[':', '*'], vec['', ''], $long)) {
          $record = substr($long, 0, 1) === '*';
          if ($record) $recorded[] = $arg;
          if (substr($long, -1, 1) === ':') {
            $i++;
            $value = $argv[$i];
            if ($record) $recorded[] = $value;
          } else {
            $value = true;
          }
          $name = str_replace(vec[':', '*', '-'], vec['', '', '_'], $long);
          $options->{$name} = $value;
          $found = true;
          break;
        }
      }

      if (!$found) {
        $msg = sprintf("Invalid argument: '%s'\nSee %s --help", $arg, $argv[0]);
        error($msg as string);
      }
    } else {
      $files[] = $arg;
    }
  }
  $options = $options as Options;

  \HH\global_set('recorded_options', $recorded);

  $repo_out = $options->repo_out;
  if ($repo_out is string && !is_dir($repo_out)) {
    if (!mkdir($repo_out) && !is_dir($repo_out)) {
      error("Unable to create repo-out dir " . $repo_out);
    }
  }
  if ($options->hhbbc2) {
    $options->repo_separate = true;
    if ($options->repo || $options->repo_single) {
      error("repo-single/repo and hhbbc2 are mutually exclusive options");
    }
    if (isset($options['mode'])) {
      error("hhbbc2 doesn't support modes; it compares hhas, doesn't run code");
    }
  }

  if ($options->repo_single || $options->repo_separate) {
    $options->repo = true;
  } else if ($options->repo) {
    // if only repo was set, then it means repo single
    $options->repo_single = true;
  }

  if ($options->jit_serialize is nonnull) {
    if (!$options->repo) {
      error("jit-serialize only works in repo mode");
    }
    if ($options->mode is nonnull && $options->mode !== 'jit') {
      error("jit-serialize only works in jit mode");
    }
  }

  if ($options->repo && $options->hhas_round_trip) {
    error("repo and hhas-round-trip are mutually exclusive options");
  }

  $multi_request_modes = multi_request_modes($options);
  if (count($multi_request_modes) > 1) {
    error("The options\n -" . implode("\n -", $multi_request_modes) .
          "\nare mutually exclusive options");
  }

  if ($options->write_to_checkout) {
    Status::$write_to_checkout = true;
  }

  return tuple($options, $files);
}

/*
 * Return the path to $test relative to $base, or false if $base does not
 * contain test.
 */
function canonical_path_from_base(string $test, string $base): mixed {
  $full = realpath($test);
  if (substr($full, 0, strlen($base)) === $base) {
    return substr($full, strlen($base) + 1);
  }
  $dirstat = stat($base);
  if (!is_dict($dirstat)) return false;
  for ($p = dirname($full); $p && $p !== "/"; $p = dirname($p)) {
    $s = stat($p);
    if (!is_dict($s)) continue;
    if ($s['ino'] === $dirstat['ino'] && $s['dev'] === $dirstat['dev']) {
      return substr($full, strlen($p) + 1);
    }
  }
  return false;
}

function canonical_path(string $test): mixed {
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
function find_test_files(string $file): vec<string>{
  $mappage = dict[
    'quick'    => 'hphp/test/quick',
    'slow'     => 'hphp/test/slow',
    'debugger' => 'hphp/test/server/debugger/tests',
    'http'     => 'hphp/test/server/http/tests',
    'fastcgi'  => 'hphp/test/server/fastcgi/tests',
    'zend'     => 'hphp/test/zend/good',
    'facebook' => 'hphp/facebook/test',

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

  return vec[$file];
}

// Some tests have to be run together in the same test bucket, serially, one
// after other in order to avoid races and other collisions.
function serial_only_tests(vec<string> $tests): vec<string> {
  if (is_testing_dso_extension()) {
    return vec[];
  }
  // Add a <testname>.php.serial file to make your test run in the serial
  // bucket.
  $serial_tests = vec(array_filter(
    $tests,
    function($test) {
      return file_exists($test . '.serial');
    }
  ));
  return $serial_tests;
}

// If "files" is very long, then the shell may reject the desired
// "find" command (especially because "escapeshellarg()" adds two single
// quote characters to each file), so we split "files" into chunks below.
function exec_find(vec<string> $files, string $extra): vec<string> {
  $results = vec[];
  foreach (array_chunk($files, 500) as $chunk) {
    $efa = implode(' ', array_map(
      $line ==> escapeshellarg($line as string),
      $chunk as dict<_, _>,
    ));
    $output = shell_exec("find $efa $extra");
    foreach (explode("\n", $output) as $result) {
      // Collect the (non-empty) results, which should all be file paths.
      if ($result !== "") $results[] = $result;
    }
  }
  return $results;
}

function is_facebook_build(Options $options): bool {
  if (!is_dir(hphp_home() . '/hphp/facebook/test')) return false;

  // We want to test for the presence of an extension in the build, so turn off
  // a bunch of features in order to do this as simply and reliably as possible.
  $simplified_options = clone $options;
  $simplified_options->repo = false;
  $simplified_options->server = false;
  $simplified_options->cli_server = false;
  // Use a bogus test name so we don't find any config overrides
  $result = runif_extension_matches(
    $simplified_options,
    'not_a_real_test.php',
    vec['facebook'],
  );
  if (!$result['valid']) {
    invariant(Shapes::keyExists($result, 'error'), 'RunifResult contract');
    invariant_violation(
      "is_facebook_build is calling runif_extension_matches incorrectly: %s",
      $result['error'],
    );
  }
  invariant(Shapes::keyExists($result, 'match'), 'RunifResult contract');
  return $result['match'];
}

function find_tests(
  vec<string> $files,
  Options $options,
): vec<string> {
  if (!$files) {
    $files = vec['quick'];
  }
  if ($files == vec['all']) {
    $files = vec['quick', 'slow', 'zend', 'fastcgi', 'http', 'debugger'];
    if (is_facebook_build($options)) {
      $files[] = 'facebook';
    }
  }
  $ft = vec[];
  foreach ($files as $file) {
    $ft = array_merge($ft, find_test_files($file));
  }
  $files = vec[];
  foreach ($ft as $file) {
    if (!@stat($file)) {
      error("Not valid file or directory: '$file'");
    }
    $file = preg_replace(',//+,', '/', realpath($file));
    $file = preg_replace(',^'.getcwd().'/,', '', $file);
    $files[] = $file;
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
    "-not -regex '.*round_trip[.]hhas' " .
    "-not -name '*.inc.php'"
  );
  if (!$tests) {
    error("Could not find any tests associated with your options.\n" .
          "Make sure your test path is correct and that you have " .
          "the right expect files for the tests you are trying to run.\n" .
          usage());
  }
  asort(inout $tests);
  $tests = vec(array_filter($tests));
  if ($options->exclude is nonnull) {
    $exclude = $options->exclude;
    $tests = vec(array_filter($tests, function($test) use ($exclude) {
      return (false === strpos($test, $exclude));
    }));
  }
  if ($options->exclude_pattern is nonnull) {
    $exclude = $options->exclude_pattern;
    $tests = vec(array_filter($tests, function($test) use ($exclude) {
      return !preg_match($exclude, $test);
    }));
  }
  if ($options->exclude_recorded_failures is nonnull) {
    $exclude_file = $options->exclude_recorded_failures;
    $exclude = file($exclude_file, FILE_IGNORE_NEW_LINES);
    $tests = vec(array_filter($tests, function($test) use ($exclude) {
      return (false === in_array(canonical_path($test), $exclude));
    }));
  }

  if ($options->list_tests) {
    $tests = vec(array_filter($tests, function($test) use ($options) {
      return should_skip_test_simple($options, $test) is null;
    }));
  }

  if ($options->include is nonnull) {
    $include = $options->include;
    $tests = vec(array_filter($tests, function($test) use ($include) {
      return (false !== strpos($test, $include));
    }));
  }
  if ($options->include_pattern is nonnull) {
    $include = $options->include_pattern;
    $tests = vec(array_filter($tests, function($test) use ($include) {
      return (bool)preg_match($include, $test);
    }));
  }
  return $tests;
}

function list_tests(vec<string> $files, Options $options): void {
  $args = implode(' ', \HH\global_get('recorded_options'));

  // Disable escaping of test info when listing. We check if the environment
  // variable is set so we can make the change in a backwards compatible way.
  $escape_info = getenv("LISTING_NO_ESCAPE") === false;

  foreach (find_tests($files, $options) as $test) {
    $test_info = Status::jsonEncode(
        dict['args' => $args, 'name' => $test],
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

function find_debug_config(string $test, string $name): string {
  $debug_config = find_file_for_dir(dirname($test), $name);
  if ($debug_config is nonnull) {
    return "-m debug --debug-config ".$debug_config;
  }
  return "";
}

function mode_cmd(Options $options): vec<string> {
  $repo_args = '';
  if (!$options->repo) {
    $repo_args = "-vUnitFileCache.Path=".unit_cache_file();
  }
  $interp_args = "$repo_args -vEval.Jit=0";
  $jit_args = "$repo_args -vEval.Jit=true";
  $mode = $options->mode ?? '';
  switch ($mode) {
    case '':
    case 'jit':
      return vec[$jit_args];
    case 'interp':
      return vec[$interp_args];
    case 'interp,jit':
      return vec[$interp_args, $jit_args];
    default:
      error("-m must be one of jit | interp | interp,jit. Got: '$mode'");
  }
}

function extra_args(Options $options): string {
  $args = $options->args ?? '';

  if ($options->vendor is nonnull) {
    $args .= ' -d auto_prepend_file=';
    $args .= escapeshellarg($options->vendor.'/hh_autoload.php');
  }

  return $args;
}

function extra_compiler_args(Options $options): string {
  return $options->compiler_args ?? '';
}

function hhvm_cmd_impl(
  Options $options,
  string $config,
  ?string $autoload_db_prefix,
  string ...$extra_args
): vec<string> {
  $cmds = vec[];
  foreach (mode_cmd($options) as $mode_num => $mode) {
    $args = vec[
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
      '-vEval.EnableLogBridge=false',
      // TODO(T115081796): Once userland autoloading is entirely ripped out we
      // should also rip this out.
      '-vAutoload.UserlandDisabled=2',
      $mode,
      $options->wholecfg ? '-vEval.JitPGORegionSelector=wholecfg' : '',

      // load/store counters don't work on Ivy Bridge so disable for tests
      '-vEval.ProfileHWEnable=false',

      // use the temp path for embedded data
      '-vEval.EmbeddedDataExtractPath='.
      Status::getRunTmpDir().
      escapeshellarg('/hhvm_%{type}_%{buildid}'),

      // Stick to a single thread for retranslate-all
      '-vEval.JitWorkerThreads=1',
      '-vEval.JitWorkerThreadsForSerdes=1',

      extra_args($options),
    ];

    if ($autoload_db_prefix is nonnull) {
      $args[] =
        '-vAutoload.DB.Path='.escapeshellarg("$autoload_db_prefix.$mode_num");
    }

    if ($options->retranslate_all is nonnull) {
      $args[] = '--count='.((int)$options->retranslate_all * 2);
      $args[] = '-vEval.JitPGO=true';
      $args[] = '-vEval.JitRetranslateAllRequest='.$options->retranslate_all;
      // Set to timeout.  We want requests to trigger retranslate all.
      $args[] = '-vEval.JitRetranslateAllSeconds=' . TIMEOUT_SECONDS;
    }

    if ($options->recycle_tc is nonnull) {
      $args[] = '--count='.$options->recycle_tc;
      $args[] = '-vEval.StressUnitCacheFreq=1';
      $args[] = '-vEval.EnableReusableTC=true';
    }

    if ($options->jit_serialize is nonnull) {
      $args[] = '-vEval.JitPGO=true';
      $args[] = '-vEval.JitRetranslateAllRequest='.$options->jit_serialize;
      // Set to timeout.  We want requests to trigger retranslate all.
      $args[] = '-vEval.JitRetranslateAllSeconds=' . TIMEOUT_SECONDS;
    }

    if ($options->hhas_round_trip) {
      $args[] = '-vEval.AllowHhas=1';
      $args[] = '-vEval.LoadFilepathFromUnitCache=1';
    }

    if ($options->verify_hackc_translator) {
      $args[] = '-vEval.VerifyTranslateHackC=1';
    }

    if (!$options->cores) {
      $args[] = '-vResourceLimit.CoreFileSize=0';
    }

    if ($options->dump_tc) {
      $args[] = '-vEval.DumpIR=2';
      $args[] = '-vEval.DumpTC=1';
    }

    if ($options->hh_single_type_check is nonnull) {
      $args[] = '--hh_single_type_check='.$options->hh_single_type_check;
    }

    if ($options->bespoke) {
      $args[] = '-vEval.BespokeArrayLikeMode=1';
      $args[] = '-vServer.APC.MemModelTreadmill=true';
    }

    $cmds[] = implode(' ', array_merge($args, $extra_args));
  }
  return $cmds;
}

function repo_separate(Options $options, string $test): bool {
  return $options->repo_separate &&
         !file_exists($test . ".hhbbc_opts");
}

// Return the command and the env to run it in.
function hhvm_cmd(
  Options $options,
  string $test,
  ?string $test_run = null,
  bool $is_temp_file = false
): (vec<string>, dict<string, mixed>) {
  $test_run ??= $test;
  // hdf support is only temporary until we fully migrate to ini
  // Discourage broad use.
  $hdf_suffix = ".use.for.ini.migration.testing.only.hdf";
  $hdf = file_exists($test.$hdf_suffix)
       ? '-c ' . $test . $hdf_suffix
       : "";
  $extra_opts = read_opts_file(find_test_ext($test, 'opts'));
  $config = find_test_ext($test, 'ini');
  invariant($config is nonnull, "%s", __METHOD__);
  $cmds = hhvm_cmd_impl(
    $options,
    $config,
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

  if ($options->cli_server) {
    $config = find_file_for_dir(dirname($test), 'config.ini');
    $servers = $options->servers as Servers;
    $server = $servers->configs[$config ?? ''];
    $socket = $server->cli_socket;
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

  if ($options->repo) {
    $repo_suffix = repo_separate($options, $test) ? 'hhbbc' : 'hhbc';

    $hhbbc_repo =
      "\"" . test_repo($options, $test) . "/hhvm.$repo_suffix\"";
    $cmd .= ' -vRepo.Authoritative=true';
    $cmd .= " -vRepo.Path=$hhbbc_repo";

    $file_cache =
      "\"" . test_repo($options, $test) . "/file.cache\"";
    $cmd .= " -vServer.FileCache=$file_cache";
  }

  if ($options->jitsample is nonnull) {
    $cmd .= ' -vDeploymentId="' . $options->jitsample . '"';
    $cmd .= ' --instance-id="' . $test . '"';
    $cmd .= ' -vEval.JitSampleRate=1';
    $cmd .= " -vScribe.Tables.hhvm_jit.include.*=instance_id";
    $cmd .= " -vScribe.Tables.hhvm_jit.include.*=deployment_id";
  }

  $env = \HH\FIXME\UNSAFE_CAST<dict<arraykey,mixed>,dict<string,mixed>>(
      \HH\global_get('_ENV') as dict<_, _>
  );
  $env['LC_ALL'] = 'C';
  $env['INPUTRC'] = test_dir().'/inputrc';

  // Apply the --env option.
  if ($options->env is nonnull) {
    foreach (explode(",", $options->env) as $arg) {
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
  if ($in is nonnull) {
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

function hphp_cmd(Options $options, string $test): string {
  // Transform extra_args like "-vName=Value" into "-vRuntime.Name=Value".
  $extra_args =
    preg_replace("/(^-v|\s+-v)\s*/", "$1Runtime.", extra_args($options));

  $compiler_args = extra_compiler_args($options);

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

  $test_repo = test_repo($options, $test);

  return implode(" ", vec[
    hphpc_path($options),
    '--hphp',
    '-vUseHHBBC='. (repo_separate($options, $test) ? 'false' : 'true'),
    '--config',
    find_test_ext($test, 'ini', 'hphp_config'),
    $hdf,
    '--repo-options-dir='.\dirname($test),
    '-vRuntime.ResourceLimit.CoreFileSize=0',
    '-vRuntime.Eval.EnableIntrinsicsExtension=true',
    // EnableArgsInBacktraces disables most of HHBBC's DCE optimizations.
    // In order to test those optimizations (which are part of a normal prod
    // configuration) we turn this flag off by default.
    '-vRuntime.Eval.EnableArgsInBacktraces=false',
    '-vRuntime.Eval.FoldLazyClassKeys=false',
    '-vRuntime.Eval.EnableLogBridge=false',
    '-vCachePHPFile=true',
    '-vParserThreadCount=' . ($options->repo_threads ?? 1),
    '-l1',
    '-o "' . $test_repo . '"',
    '--file-cache="'.$test_repo.'/file.cache"',
    "\"$test\"",
    "-vExternWorker.WorkingDir=".Status::getTestTmpPath($test, 'work'),
    $extra_args,
    $compiler_args,
    read_opts_file(find_test_ext($test, 'hphp_opts')),
  ]);
}

function hphpc_path(Options $options): string {
  return hhvm_path();
}

function hhbbc_cmd(Options $options, string $test): string {
  $test_repo = test_repo($options, $test);
  return implode(" ", vec[
    hphpc_path($options),
    '--hhbbc',
    '--no-logging',
    '--no-cores',
    '--parallel-num-threads=' . ($options->repo_threads ?? 1),
    '--parallel-final-threads=' . ($options->repo_threads ?? 1),
    '--extern-worker-working-dir=' . Status::getTestTmpPath($test, 'work'),
    read_opts_file("$test.hhbbc_opts"),
    "-o \"$test_repo/hhvm.hhbbc\" \"$test_repo/hhvm.hhbc\"",
  ]);
}

// Execute $cmd and return its output on failure, including any stacktrace.log
// file it generated. Return null on success.
function exec_with_stack(string $cmd): ?string {
  $pipes = null;
  $proc = proc_open($cmd,
                    dict[0 => vec['pipe', 'r'],
                          1 => vec['pipe', 'w'],
                          2 => vec['pipe', 'w']], inout $pipes);
  $pipes as nonnull;
  fclose($pipes[0]);
  $s = '';
  $all_selects_failed=true;
  $end = mtime() + TIMEOUT_SECONDS;
  $timedout = false;
  while (true) {
    $now = mtime();
    if ($now >= $end) break;
    $read = vec[$pipes[1], $pipes[2]];
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
    $read as nonnull;
    foreach ($read as $pipe) {
      $t = fread($pipe, 4096);
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
    $now = mtime();
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
    return null;
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

function repo_mode_compile(Options $options, string $test): bool {
  $hphp = hphp_cmd($options, $test);
  $result = exec_with_stack($hphp);
  if ($result is null && repo_separate($options, $test)) {
    $hhbbc = hhbbc_cmd($options, $test);
    $result = exec_with_stack($hhbbc);
  }
  if ($result is null) return true;
  Status::writeDiff($test, $result);
  return false;
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
// The first call to "getInput()" or "getOutput()" in any process will
// block until some other process calls the other method.
//
class Queue {
  // The path to the FIFO, until destroyed.
  private ?string $path = null;

  private ?resource $input = null;
  private ?resource $output = null;

  // Pipes writes are atomic up to 512 bytes (up to 4096 bytes on linux),
  // and we use a 16 byte header, leaving this many bytes available for
  // each chunk of "body" (see "$partials").
  const int CHUNK = 512 - 16;

  // If a message "body" is larger than CHUNK bytes, then writers must break
  // it into chunks, and send all but the last chunk with type 0.  The reader
  // collects those chunks in this Map (indexed by pid), until the final chunk
  // is received, and the chunks can be reassembled.
  private Map<int, Vector<string>> $partials = Map {};

  public function __construct(?string $dir = null): void {
    $path = \tempnam($dir ?? \sys_get_temp_dir(), "queue.mkfifo.");
    \unlink($path);
    if (!\posix_mkfifo($path, 0700)) {
      // Only certain directories support "posix_mkfifo()".
      throw new \Exception("Failed to create FIFO at '$path'");
    }
    $this->path = $path;
  }

  private function getInput(): resource {
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

  private function getOutput(): resource {
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
          $body = \implode("", $chunks);
          $this->partials->removeKey($pid);
        }
        return tuple($pid, $type, $body);
      }
    }
  }

  // Receive one message (pid, type, message).
  // Note that the raw body is processed using "unserialize()".
  public function receiveMessage(): (int, int, ?Message) {
    list($pid, $type, $body) = $this->receive();
    $msg = unserialize($body) as ?Message;
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
    // Hack's "fwrite()" is never buffered, which is especially
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

  // Send one serialized message.
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

  // Send one message after serializing it.
  public function sendMessage(int $type, ?Message $msg): void {
    $body = serialize($msg);
    $this->send($type, $body);
  }

  public function destroy(): void {
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

final class Message {
  public function __construct(
    public string $test,
    public float $time,
    public int $stime,
    public int $etime,
    public ?string $reason = null,
  ) {
  }
}

enum TempDirRemove: int {
  ALWAYS = 0;
  ON_RUN_SUCCESS = 1;
  NEVER = 2;
}

type TestResult = shape(
  'name' => string,
  'status' => string,
  'start_time' => int,
  'end_time' => int,
  'time' => float,
  ?'details' => string,
);

final class Status {
  private static vec<TestResult> $results = vec[];
  private static int $mode = 0;

  private static bool $use_color = false;

  public static bool $nofork = false;
  private static ?Queue $queue = null;
  private static bool $killed = false;
  public static TempDirRemove $temp_dir_remove = TempDirRemove::ALWAYS;
  private static int $return_value = 255;

  private static float $overall_start_time = 0.0;
  private static float $overall_end_time = 0.0;

  private static string $tmpdir = "";
  public static bool $write_to_checkout = false;

  public static int $passed = 0;
  public static int $skipped = 0;
  public static dict<string, int> $skip_reasons = dict[];
  public static int $failed = 0;

  const int MODE_NORMAL = 0;
  const int MODE_VERBOSE = 1;
  const int MODE_TESTPILOT = 3;
  const int MODE_RECORD_FAILURES = 4;

  const int MSG_STARTED = 7;
  const int MSG_FINISHED = 1;
  const int MSG_TEST_PASS = 2;
  const int MSG_TEST_FAIL = 4;
  const int MSG_TEST_SKIP = 5;
  const int MSG_SERVER_RESTARTED = 6;

  const int RED = 31;
  const int GREEN = 32;
  const int YELLOW = 33;
  const int BLUE = 34;

  public static function createTmpDir(?string $working_dir): void {
    $parent = $working_dir ?? sys_get_temp_dir();
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

  public static function removeDirectory(string $dir): void {
    $files = scandir($dir);
    foreach ($files as $file) {
      if ($file === '.' || $file === '..') {
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
  public static function removeEmptyTestParentDirs(string $dir): bool {
    $is_now_empty = true;
    $files = scandir($dir);
    foreach ($files as $file) {
      if ($file === '.' || $file === '..') {
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

  public static function setMode(int $mode): void {
    self::$mode = $mode;
  }

  public static function getMode(): int {
    return self::$mode;
  }

  public static function setUseColor(bool $use): void {
    self::$use_color = $use;
  }

  public static function addTestTimesSerial(
    dict<string, TestResult> $results,
  ): float {
    $time = 0.0;
    foreach ($results as $result) {
      $time += $result['time'];
    }
    return $time;
  }

  public static function getOverallStartTime(): float {
    return self::$overall_start_time;
  }

  public static function getOverallEndTime(): float {
    return self::$overall_end_time;
  }

  public static function started(): void {
    self::send(self::MSG_STARTED, null);
    self::$overall_start_time = mtime();
  }

  public static function finished(int $return_value): void {
    self::$overall_end_time = mtime();
    self::$return_value = $return_value;
    self::send(self::MSG_FINISHED, null);
  }

  public static function destroy(): void {
    if (!self::$killed) {
      self::$killed = true;
      if (self::$queue is nonnull) {
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

  public static function destroyFromSignal(int $_signo): void {
    self::destroy();
  }

  public static function registerCleanup(bool $no_clean): void {
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

  public static function serverRestarted(): void {
    self::send(self::MSG_SERVER_RESTARTED, null);
  }

  public static function pass(
    string $test, float $time, int $stime, int $etime,
  ): void {
    self::$results[] = shape(
      'name' => $test,
      'status' => 'passed',
      'start_time' => $stime,
      'end_time' => $etime,
      'time' => $time
    );
    self::send(
      self::MSG_TEST_PASS,
      new Message($test, $time, $stime, $etime),
    );
  }

  public static function skip(
    string $test, string $reason, float $time, int $stime, int $etime,
  ): void {
    self::$results[] = shape(
      'name' => $test,
      /* testpilot needs a positive response for every test run, report
       * that this test isn't relevant so it can silently drop. */
      'status' => self::getMode() === self::MODE_TESTPILOT
        ? 'not_relevant'
        : 'skipped',
      'start_time' => $stime,
      'end_time' => $etime,
      'time' => $time,
    );
    self::send(
      self::MSG_TEST_SKIP,
      new Message($test, $time, $stime, $etime, $reason),
    );
  }

  public static function fail(
    string $test, float $time, int $stime, int $etime, string $diff,
  ): void {
    self::$results[] = shape(
      'name' => $test,
      'status' => 'failed',
      'start_time' => $stime,
      'end_time' => $etime,
      'time' => $time,
      'details' => self::utf8Sanitize($diff),
    );
    self::send(
      self::MSG_TEST_FAIL,
      new Message($test, $time, $stime, $etime),
    );
  }

  public static function handle_message(int $type, ?Message $message): bool {
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
            Status::sayColor(
              Status::YELLOW,
              "failed to talk to server\n"
            );
            break;

          case Status::MODE_TESTPILOT:
            break;

          case Status::MODE_RECORD_FAILURES:
            break;
        }
        break;

      case Status::MSG_TEST_PASS:
        self::$passed++;
        invariant($message is nonnull, "%s", __METHOD__);
        switch (Status::getMode()) {
          case Status::MODE_NORMAL:
            if (!Status::hasCursorControl()) {
              Status::sayColor(Status::GREEN, '.');
            }
            break;

          case Status::MODE_VERBOSE:
            Status::sayColor(
              $message->test." ",
              Status::GREEN,
              sprintf("passed (%.2fs)\n", $message->time),
            );
            break;

          case Status::MODE_TESTPILOT:
            Status::sayTestpilot(
              $message->test,
              'passed',
              $message->stime,
              $message->etime,
              $message->time,
            );
            break;

          case Status::MODE_RECORD_FAILURES:
            break;
        }
        break;

      case Status::MSG_TEST_SKIP:
        self::$skipped++;
        invariant($message is nonnull, "%s", __METHOD__);
        $reason = $message->reason;
        invariant($reason is nonnull, "%s", __METHOD__);
        self::$skip_reasons[$reason] ??= 0;
        self::$skip_reasons[$reason]++;

        switch (Status::getMode()) {
          case Status::MODE_NORMAL:
            if (!Status::hasCursorControl()) {
              Status::sayColor(Status::YELLOW, 's');
            }
            break;

          case Status::MODE_VERBOSE:
            Status::sayColor($message->test." ", Status::YELLOW, "skipped");

            if ($reason is nonnull) {
              Status::sayColor(" - reason: $reason");
            }
            Status::sayColor(sprintf(" (%.2fs)\n", $message->time));
            break;

          case Status::MODE_TESTPILOT:
            $skip_msg = $message->test." skipped (by run.php)";
            if ($reason is nonnull) {
              $skip_msg .= " - reason: $reason";
            }
            Status::sayTestpilot(
              $message->test,
              'not_relevant',
              $message->stime,
              $message->etime,
              $message->time,
              $skip_msg,
            );
            break;

          case Status::MODE_RECORD_FAILURES:
            break;
        }
        break;

      case Status::MSG_TEST_FAIL:
        self::$failed++;
        invariant($message is nonnull, "%s", __METHOD__);
        switch (Status::getMode()) {
          case Status::MODE_NORMAL:
            if (Status::hasCursorControl()) {
              print "\033[2K\033[1G";
            }
            $diff = Status::diffForTest($message->test);
            $test = $message->test;
            Status::sayColor(
              Status::RED,
              "\nFAILED: $test\n$diff\n",
            );
            break;

          case Status::MODE_VERBOSE:
            Status::sayColor(
              $message->test." ",
              Status::RED,
              sprintf("FAILED (%.2fs)\n", $message->time),
            );
            break;

          case Status::MODE_TESTPILOT:
            Status::sayTestpilot(
              $message->test,
              'failed',
              $message->stime,
              $message->etime,
              $message->time,
            );
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

  private static function send(int $type, ?Message $msg): void {
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
    * Takes a variable number of string or int arguments. If color output is
    * enabled and any one of the arguments is preceded by an integer (see the
    * color constants above), that argument will be given the indicated color.
   */
  public static function sayColor(arraykey ...$args): void {
    $n = count($args);
    for ($i = 0; $i < $n;) {
      $arg = $args[$i];
      $i++;
      if ($arg is int) {
        $color = $arg;
        if (self::$use_color) {
          print "\033[0;{$color}m";
        }
        $arg = $args[$i];
        $i++;
        print $arg;
        if (self::$use_color) {
          print "\033[0m";
        }
      } else {
        print $arg;
      }
    }
  }

  public static function sayTestpilot(
      string $test, string $status, int $stime, int $etime, float $time,
      ?string $msg = null,
  ): void {
    $start = dict['op' => 'start', 'test' => $test];
    $end = dict['op' => 'test_done', 'test' => $test, 'status' => $status,
                 'start_time' => $stime, 'end_time' => $etime, 'time' => $time];
    if ($status === 'failed') {
      $end['details'] = self::utf8Sanitize(Status::diffForTest($test));
    } else if ($msg is nonnull) {
      $end['details'] = self::utf8Sanitize($msg);
    }
    self::say($start, $end);
  }

  public static function getResults(): vec<TestResult> {
    return self::$results;
  }

  /** Output is in the format expected by JsonTestRunner. */
  public static function say(dict<string, mixed> ...$args): void {
    $data = array_map(
      $row ==> self::jsonEncode($row) . "\n",
      $args
    );
    fwrite(STDERR, implode("", $data));
  }

  <<__Memoize>>
  public static function hasCursorControl(): bool {
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
    // emacs hack-mode really doesn't like <undef> inside a string...
    return strpos($stty, 'erase = ' . '<' . 'undef' . '>') === false;
  }

  <<__Memoize>>
  public static function getSTTY(): string {
    $descriptorspec = dict[1 => vec["pipe", "w"], 2 => vec["pipe", "w"]];
    $pipes = null;
    $process = proc_open(
      'stty -a', $descriptorspec, inout $pipes, null, null,
      dict['suppress_errors' => true]
    );
    $pipes as nonnull;
    $stty = stream_get_contents($pipes[1]);
    proc_close($process);
    return $stty;
  }

  public static function utf8Sanitize(string $str): string {
    return UConverter::transcode($str, 'UTF-8', 'UTF-8');
  }

  public static function jsonEncode(mixed $data): string {
    return json_encode($data, JSON_UNESCAPED_SLASHES);
  }

  public static function getQueue(): Queue {
    if (!self::$queue) {
      if (self::$killed) error("Killed!");
      self::$queue = new Queue(self::$tmpdir);
    }
    return self::$queue;
  }
}

function clean_intermediate_files(string $test, Options $options): void {
  if ($options->no_clean) {
    return;
  }
  if ($options->write_to_checkout) {
    // in --write-to-checkout mode, normal test output goes next to the test
    $exts = vec[
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
  $tmp_exts = vec[
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
  Options $options,
  vec<string> $tests,
  string $json_results_file,
): int {
  foreach ($tests as $test) {
    run_and_log_test($options, $test);
  }
  $results = Status::getResults();
  file_put_contents($json_results_file, json_encode($results));
  foreach ($results as $result) {
    if ($result['status'] === 'failed') {
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

<<__Memoize>>
function runif_canonical_os(): string {
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

function runif_os_matches(vec<string> $words): RunifResult {
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

function runif_file_matches(vec<string> $words): RunifResult {
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
  Options $options,
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
  $options_without_repo = clone $options;
  $options_without_repo->repo = false;
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
    "unexpected output from shell_exec in runif_test_for_feature: '%s'",
    $result
  );
}

function runif_euid_matches(
  Options $options,
  string $test,
  vec<string> $words,
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
  Options $options,
  string $test,
  vec<string> $words,
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
  Options $options,
  string $test,
  vec<string> $words,
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
  Options $options,
  string $test,
  vec<string> $words,
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
  Options $options,
  string $test,
  vec<string> $words,
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
  Options $options,
  string $test,
  vec<string> $words,
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
  Options $options,
  string $test,
  vec<string> $words,
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
  Options $options,
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
    $words = vec($words); // array_shift always promotes to dict :-\
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
    if (!$result['valid'] || !Shapes::idx($result, 'match', false)) {
      return $result;
    }
  }
  if ($file_empty) return shape('valid' => false, 'error' => 'empty runif file');
  return shape('valid' => true, 'match' => true);
}

// should_skip_test_simple handles generating skips in ways that are purely
// based on options passed to run.php, and test names/exclusion files (eg.
// norepo).  The benefit of should_skip_test_simple is that it can factor in to
// test listing, while more complex skip behavior will appear in test results.
function should_skip_test_simple(
  Options $options,
  string $test,
): ?string {
  if (($options->cli_server || $options->server) &&
      !can_run_server_test($test, $options)) {
    return 'skip-server';
  }

  if ($options->hhas_round_trip && substr($test, -5) === ".hhas") {
    return 'skip-hhas';
  }

  if ($options->hhbbc2 || $options->hhas_round_trip) {
    $no_hhas_tag = 'nodumphhas';
    if (file_exists("$test.$no_hhas_tag") ||
        file_exists(dirname($test).'/'.$no_hhas_tag)) {
      return 'skip-nodumphhas';
    }
    if (file_exists($test . ".verify")) {
      return 'skip-verify';
    }
  }

  if (has_multi_request_mode($options) || $options->repo ||
      $options->server) {
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
  if ($options->bespoke &&
      file_exists("$test.$no_bespoke_tag")) {
      // Skip due to changes in array identity
      return 'skip-bespoke';
  }

  $no_jitserialize_tag = "nojitserialize";
  if ($options->jit_serialize is nonnull &&
      file_exists("$test.$no_jitserialize_tag")) {
    return 'skip-jit-serialize';
  }

  if ((!$options->repo || $options->jit_serialize is null
       || (int)$options->jit_serialize < 1)
      && file_exists($test.'.onlyjumpstart')) {
    return 'skip-onlyjumpstart';
  }

  if (!$options->repo && file_exists($test.'.onlyrepo')) {
    return 'skip-onlyrepo';
  }

  if ($options->repo && file_exists($test.'.norepo')) {
    return 'skip-norepo';
  }

  if ($options->only_remote_executable && file_exists("$test.no_remote")) {
    return 'skip-only_running_remote_executable_tests';
  }
  if ($options->only_non_remote_executable && !file_exists("$test.no_remote")) {
    return 'skip-only_running_NON_remote_executable_tests';
  }

  return null;
}

function skipif_should_skip_test(
  Options $options,
  string $test,
): RunifResult {
  $skipif_test = find_test_ext($test, 'skipif');
  if (!$skipif_test) {
    return shape('valid' => true, 'match' => true);
  }

  // Run the .skipif in non-repo mode since building a repo for it is
  // inconvenient and the same features should be available. Pick the mode
  // arbitrarily for the same reason.
  $options_without_repo = clone $options;
  $options_without_repo->repo = false;
  list($hhvm, $_) = hhvm_cmd($options_without_repo, $test, $skipif_test);
  $hhvm = $hhvm[0];
  // Remove any --count <n> from the command
  $hhvm = preg_replace('/ --count[ =]\d+/', '', $hhvm);

  $descriptorspec = dict[
    0 => vec["pipe", "r"],
    1 => vec["pipe", "w"],
    2 => vec["pipe", "w"],
  ];
  $pipes = null;
  $process = proc_open("$hhvm $test 2>&1", $descriptorspec, inout $pipes);
  if (!is_resource($process)) {
    return shape(
      'valid' => false,
      'error' => 'proc_open failed while running skipif'
    );
  }

  $pipes as nonnull;
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

class BadExpectfPattern extends Exception {
  public function __construct(string $str, string $pattern, int $offset) {
    parent::__construct(
      "$str (at offset $offset, context: \"" .
      substr($pattern, max($offset - 25, 0), 50) .
      "\")"
    );
  }
}

class ExpectfParser {
  private string $str;

  private string $pstr;
  private vec<?(function(int): vec<(int, int)>)> $pattern = vec[];
  private int $pindex = 0;

  private bool $found_wildcard = false;
  private bool $success = false;

  public function __construct(string $str, string $pattern) {
    $this->str = $str;
    $this->pstr = $pattern;
    $this->parse();
    $this->success = $this->run();
  }

  public function succeeded(): bool { return $this->success; }
  public function foundWildcard(): bool { return $this->found_wildcard; }

  private function literal(string $s): void {
    $next = count($this->pattern) + 1;
    $this->pattern[] = (int $sindex) ==> {
      if (!strlen($s)) return vec[tuple($sindex, $next)];
      if ($sindex + strlen($s) > strlen($this->str)) return vec[];
      if (substr_compare($this->str, $s, $sindex, strlen($s)) !== 0) {
        return vec[];
      }
      return vec[tuple($sindex + strlen($s), $next)];
    };
  }

  private function any(): void {
    $next = count($this->pattern) + 1;
    $this->pattern[] = (int $sindex)  ==> {
      if ($sindex === strlen($this->str)) return vec[];
      return vec[tuple($sindex + 1, $next)];
    };
  }

  private function anyBut(string $s): void {
    $next = count($this->pattern) + 1;
    $this->pattern[] = (int $sindex) ==> {
      if ($sindex === strlen($this->str)) return vec[];
      if ($s === $this->str[$sindex]) return vec[];
      return vec[tuple($sindex + 1, $next)];
    };
  }

  private function func((function(string): bool) $f): void {
    $next = count($this->pattern) + 1;
    $this->pattern[] = (int $sindex) ==> {
      if ($sindex === strlen($this->str)) return vec[];
      if (!$f($this->str[$sindex])) return vec[];
      return vec[tuple($sindex + 1, $next)];
    };
  }

  private function or(vec<(function(): void)> $choices): void {
    $start = count($this->pattern);
    $this->pattern[] = null;

    $choice_indices = vec[];
    $jmp_indices = vec[];
    foreach ($choices as $c) {
      $choice_indices[] = count($this->pattern);
      $c();
      $jmp_indices[] = count($this->pattern);
      $this->pattern[] = null;
    }
    $end = count($this->pattern);

    $this->pattern[$start] = (int $sindex) ==> {
      $v = vec[];
      foreach ($choice_indices as $i) $v[] = tuple($sindex, $i);
      return $v;
    };

    foreach ($jmp_indices as $i) {
      $this->pattern[$i] = (int $sindex) ==> {
        return vec[tuple($sindex, $end)];
      };
    }
  }

  private function orSub(): void {
    $start = count($this->pattern);
    $this->pattern[] = null;

    $choice_indices = vec[];
    $jmp_indices = vec[];
    $finished = false;
    do {
      $choice_indices[] = count($this->pattern);
      $finished = $this->parse('|');
      $jmp_indices[] = count($this->pattern);
      $this->pattern[] = null;
    } while (!$finished);
    $end = count($this->pattern);

    $this->pattern[$start] = (int $sindex) ==> {
      $v = vec[];
      foreach ($choice_indices as $i) $v[] = tuple($sindex, $i);
      return $v;
    };

    foreach ($jmp_indices as $i) {
      $this->pattern[$i] = (int $sindex) ==> {
        return vec[tuple($sindex, $end)];
      };
    }
  }

  private function zeroOrMore((function(): void) $c): void {
    $index1 = count($this->pattern);
    $this->pattern[] = null;
    $c();
    $this->pattern[] = (int $sindex) ==> {
      return vec[tuple($sindex, $index1)];
    };
    $index2 = count($this->pattern);
    $this->pattern[$index1] = (int $sindex) ==> {
      return vec[tuple($sindex, $index1+1), tuple($sindex, $index2)];
    };
  }

  private function oneOrMore((function(): void) $c): void {
    $start = count($this->pattern);
    $c();
    $end = count($this->pattern);
    $this->pattern[] = (int $sindex) ==> {
      return vec[tuple($sindex, $start), tuple($sindex, $end + 1)];
    };
  }

  private function opt((function(): void) $c): void {
    $index1 = count($this->pattern);
    $this->pattern[] = null;
    $c();
    $index2 = count($this->pattern);
    $this->pattern[$index1] = (int $sindex) ==> {
      return vec[tuple($sindex, $index1+1), tuple($sindex, $index2)];
    };
  }

  private function plusMinus(): void {
    $this->or(vec[() ==> $this->literal('-'), () ==> $this->literal('+')]);
  }

  private function whitespace(): void {
    $this->func(ctype_space<>);
  }
  private function digit(): void {
    $this->func(ctype_digit<>);
  }
  private function hexdigit(): void {
    $this->func(ctype_xdigit<>);
  }
  private function notnewline(): void {
    $this->anyBut("\n");
  }

  private function parse(?string $sub = null): bool {
    $current_literal = '';

    $size = strlen($this->pstr);
    while ($this->pindex < $size) {
      $token = $this->pstr[$this->pindex];
      ++$this->pindex;
      if ($token !== '%') {
        if ($sub is nonnull) {
          if ($token === '}') {
            if (strlen($current_literal)) $this->literal($current_literal);
            return true;
          }
          if ($token === '|' && $sub === '|') {
            if (strlen($current_literal)) $this->literal($current_literal);
            return false;
          }
        }
        $current_literal .= $token;
        continue;
      }

      if ($this->pindex == $size) {
        throw new BadExpectfPattern(
          "Unterminated wildcard at end of pattern", $this->pstr, $this->pindex
        );
      }
      $token = $this->pstr[$this->pindex];
      ++$this->pindex;

      if ($token !== '%' && $token !== 't' && $token !== 'h') {
        $this->found_wildcard = true;
        if (strlen($current_literal)) {
          $this->literal($current_literal);
          $current_literal = '';
        }
      }

      switch ($token) {
        case '%':
          $current_literal .= '%';
          break;
        case 't':
          $current_literal .= "\t";
          break;
        case 'c':
          $this->any();
          break;
        case 'd':
          $this->oneOrMore(() ==> $this->digit());
          break;
        case 'x':
          $this->oneOrMore(() ==> $this->hexdigit());
          break;
        case 's':
          $this->oneOrMore(() ==> $this->notnewline());
          break;
        case 'a':
          $this->oneOrMore(() ==> $this->any());
          break;
        case 'w':
          $this->zeroOrMore(() ==> $this->whitespace());
          break;
        case 'C':
          $this->opt(() ==> $this->any());
          break;
        case 'S':
          $this->zeroOrMore(() ==> $this->notnewline());
          break;
        case 'A':
          $this->zeroOrMore(() ==> $this->any());
          break;
        case 'i':
          $this->opt(() ==> $this->plusMinus());
          $this->oneOrMore(() ==> $this->digit());
          break;
        case 'f':
          // This is more permissive than necessary, but good enough.
          $this->opt(() ==> $this->plusMinus());
          $this->opt(() ==> $this->literal('.'));
          $this->oneOrMore(() ==> $this->digit());
          $this->opt(() ==> $this->literal('.'));
          $this->zeroOrMore(() ==> $this->digit());
          $this->opt(() ==> {
            $this->or(
              vec[() ==> $this->literal('E'), () ==> $this->literal('e')]
            );
            $this->opt(() ==> $this->plusMinus());
            $this->oneOrMore(() ==> $this->digit());
          });
          break;
        case 'h':
          if ($this->pindex == $size) {
            throw new BadExpectfPattern(
              "While parsing %h, expected '{', but reached end of pattern",
              $this->pstr, $this->pindex
            );
          }
          $brace = $this->pstr[$this->pindex];
          if ($brace !== '{') {
            throw new BadExpectfPattern(
              "While parsing %h, expected '{', but got '$brace'",
              $this->pstr, $this->pindex
            );
          }
          ++$this->pindex;
          if ($this->pindex == $size) {
            throw new BadExpectfPattern(
              "While parsing %h, expected hex digit, but reached end of pattern",
              $this->pstr, $this->pindex
            );
          }
          $h1 = $this->pstr[$this->pindex];
          if (!ctype_xdigit($h1)) {
            throw new BadExpectfPattern(
              "While parsing %h, expected hex digit, but got '$h1'",
              $this->pstr, $this->pindex
            );
          }
          ++$this->pindex;
          if ($this->pindex == $size) {
            throw new BadExpectfPattern(
              "While parsing %h, expected hex digit, but reached end of pattern",
              $this->pstr, $this->pindex
            );
          }
          $h2 = $this->pstr[$this->pindex];
          if (!ctype_xdigit($h2)) {
            throw new BadExpectfPattern(
              "While parsing %h, expected hex digit, but got '$h2'",
              $this->pstr, $this->pindex
            );
          }
          ++$this->pindex;
          if ($this->pindex == $size) {
            throw new BadExpectfPattern(
              "While parsing %h, expected '}', but reached end of pattern",
              $this->pstr, $this->pindex
            );
          }
          $brace = $this->pstr[$this->pindex];
          if ($brace !== '}') {
            throw new BadExpectfPattern(
              "While parsing %h, expected '}', but got '$brace'",
              $this->pstr, $this->pindex
            );
          }
          ++$this->pindex;
          $current_literal .= chr(hexdec($h1 . $h2));
          break;
        case '|':
          if ($this->pindex == $size) {
            throw new BadExpectfPattern(
              "While parsing %|, expected '{', but reached end of pattern",
              $this->pstr, $this->pindex
            );
          }
          $brace = $this->pstr[$this->pindex];
          if ($brace !== '{') {
            throw new BadExpectfPattern(
              "While parsing %|, expected '{', but got '$brace'",
              $this->pstr, $this->pindex
            );
          }
          ++$this->pindex;
          $this->orSub();
          break;
        case '?':
          if ($this->pindex == $size) {
            throw new BadExpectfPattern(
              "While parsing %?, expected '{', but reached end of pattern",
              $this->pstr, $this->pindex
            );
          }
          $brace = $this->pstr[$this->pindex];
          if ($brace !== '{') {
            throw new BadExpectfPattern(
              "While parsing %?, expected '{', but got '$brace'",
              $this->pstr, $this->pindex
            );
          }
          ++$this->pindex;
          $this->opt(() ==> { $this->parse('?'); return; });
          break;
        case '*':
          if ($this->pindex == $size) {
            throw new BadExpectfPattern(
              "While parsing %*, expected '{', but reached end of pattern",
              $this->pstr, $this->pindex
            );
          }
          $brace = $this->pstr[$this->pindex];
          if ($brace !== '{') {
            throw new BadExpectfPattern(
              "While parsing %*, expected '{', but got '$brace'",
              $this->pstr, $this->pindex
            );
          }
          ++$this->pindex;
          $this->zeroOrMore(() ==> { $this->parse('*'); return; });
          break;
        default:
          throw new BadExpectfPattern(
            "Unknown wildcard %$token (escape with % if not a wildcard)",
            $this->pstr, $this->pindex
          );
      }
    }

    if ($sub is nonnull) {
      throw new BadExpectfPattern(
        "While parsing %$sub, expected '}', but reached end of pattern",
        $this->pstr, $this->pindex
      );
    }
    if (strlen($current_literal)) $this->literal($current_literal);
    return false;
  }

  private function run(): bool {
    $states = dict[];
    $states[0] = keyset[];
    $states[0][] = 0;

    do {
      $newStates = dict[];
      foreach ($states as $pindex => $sindices) {
        foreach ($sindices as $sindex) {
          if ($pindex === count($this->pattern)) {
            if ($sindex === strlen($this->str)) return true;
            continue;
          }
          foreach (($this->pattern[$pindex] as nonnull)($sindex) as list($s, $p)) {
            $newStates[$p] ??= keyset[];
            $newStates[$p][] = $s;
          }
        }
      }
      $states = $newStates;
    } while ($states);
    return false;
  }
}

function count_array_diff(
  vec<string> $ar1, vec<string> $ar2,
  int $idx1, int $idx2, int $cnt1, int $cnt2, num $steps,
  (function(string, string): bool) $cmp
): int {
  $equal = 0;

  while ($idx1 < $cnt1 && $idx2 < $cnt2 && $cmp($ar1[$idx1], $ar2[$idx2])) {
    $idx1++;
    $idx2++;
    $equal++;
    $steps--;
  }
  $steps--;
  if ($steps > 0) {
    $eq1 = 0;
    $st = $steps / 2;

    for ($ofs1 = $idx1 + 1; $ofs1 < $cnt1 && $st > 0; $ofs1++) {
      $st--;
      $eq = @count_array_diff($ar1, $ar2, $ofs1, $idx2, $cnt1,
                              $cnt2, $st, $cmp);

      if ($eq > $eq1) {
        $eq1 = $eq;
      }
    }

    $eq2 = 0;
    $st = $steps;

    for ($ofs2 = $idx2 + 1; $ofs2 < $cnt2 && $st > 0; $ofs2++) {
      $st--;
      $eq = @count_array_diff($ar1, $ar2, $idx1, $ofs2, $cnt1,
                              $cnt2, $st, $cmp);
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

function generate_array_diff(
  vec<string> $ar1,
  vec<string> $ar2,
  vec<string> $w,
  (function(string, string): bool) $cmp
): vec<string> {
  $idx1 = 0; $cnt1 = @count($ar1);
  $idx2 = 0; $cnt2 = @count($ar2);
  $old1 = dict[];
  $old2 = dict[];

  while ($idx1 < $cnt1 && $idx2 < $cnt2) {
    if ($cmp($ar1[$idx1], $ar2[$idx2])) {
      $idx1++;
      $idx2++;
      continue;
    } else {
      $c1 = @count_array_diff($ar1, $ar2, $idx1+1, $idx2, $cnt1,
                              $cnt2, 10, $cmp);
      $c2 = @count_array_diff($ar1, $ar2, $idx1, $idx2+1, $cnt1,
                              $cnt2, 10, $cmp);

      if ($c1 > $c2) {
        $old1[$idx1+1] = sprintf("%03d- ", $idx1+1) . $w[$idx1];
        $idx1++;
      } else if ($c2 > 0) {
        $old2[$idx2+1] = sprintf("%03d+ ", $idx2+1) . $ar2[$idx2];
        $idx2++;
      } else {
        $old1[$idx1+1] = sprintf("%03d- ", $idx1+1) . $w[$idx1];
        $old2[$idx2+1] = sprintf("%03d+ ", $idx2+1) . $ar2[$idx2];
        $idx1++;
        $idx2++;
      }
    }
  }

  $diff = vec[];
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
    $k1 = $iter1 < $end1 ? $old1_keys[$iter1] : -2;
    $k2 = $iter2 < $end2 ? $old2_keys[$iter2] : -2;
    if ($k1 === $l1 + 1 || $iter2 >= $end2) {
      $l1 = $k1;
      $diff[] = $old1_values[$iter1];
      $iter1++;
    } else if ($k2 === $l2 + 1 || $iter1 >= $end1) {
      $l2 = $k2;
      $diff[] = $old2_values[$iter2];
      $iter2++;
    } else if ($k1 < $k2) {
      $l1 = $k1;
      $diff[] = $old1_values[$iter1];
      $iter1++;
    } else {
      $l2 = $k2;
      $diff[] = $old2_values[$iter2];
      $iter2++;
    }
  }

  while ($idx1 < $cnt1) {
    $diff[] = sprintf("%03d- ", $idx1 + 1) . $w[$idx1];
    $idx1++;
  }

  while ($idx2 < $cnt2) {
    $diff[] = sprintf("%03d+ ", $idx2 + 1) . $ar2[$idx2];
    $idx2++;
  }

  return $diff;
}

function escape_unprintables(string $str): string {
  $out = '';
  for ($i = 0; $i < strlen($str); $i++) {
    $s = $str[$i];
    if (ctype_print($s)) {
      $out .= $s;
    } else if ($s === "\n") {
      $out .= '\n';
    } else if ($s === "\r") {
      $out .= '\r';
    } else if ($s === "\t") {
      $out .= '\t';
    } else {
      $h = dechex(ord($s));
      if (strlen($h) < 2) $h = "0" . $h;
      $out .= '\x' . $h;
    }
  }
  return $out;
}

function generate_diff(
  string $wanted,
  string $output,
  (function(string, string): bool) $cmp
): string {
  $w = explode("\n", $wanted);
  $o = explode("\n", $output);
  $diff = generate_array_diff($w, $o, $w, $cmp);
  if (count($diff) > 200) {
    $diff = array_slice($diff, 0, 200);
    $diff[] = "(truncated)";
  }
  return implode("\n", array_map(escape_unprintables<>, $diff));
}

function dump_hhas_cmd(
  string $hhvm_cmd, string $test, string $hhas_file,
): string {
  $dump_flags = implode(' ', vec[
    '-vEval.AllowHhas=true',
    '-vEval.DumpHhas=1',
    '-vEval.DumpHhasToFile='.escapeshellarg($hhas_file),
    '-vEval.LoadFilepathFromUnitCache=0',
  ]);
  $cmd = str_replace(' -- ', " $dump_flags -- ", $hhvm_cmd);
  if ($cmd === $hhvm_cmd) $cmd .= " $dump_flags";
  return $cmd;
}

function dump_hhas_to_temp(string $hhvm_cmd, string $test): ?string {
  $temp_file = Status::getTestTmpPath($test, 'round_trip.hhas');
  $cmd = dump_hhas_cmd($hhvm_cmd, $test, $temp_file);
  $ret = -1;
  system("$cmd &> /dev/null", inout $ret);
  return $ret === 0 ? $temp_file : null;
}

const vec<string> SERVER_EXCLUDE_PATHS = vec[
  'quick/xenon/',
  'slow/streams/',
  'slow/ext_mongo/',
  'slow/ext_oauth/',
  'slow/ext_vsdebug/',
  'zend/good/ext/standard/tests/array/',
];

const string HHAS_EXT = '.hhas';

function can_run_server_test(string $test, Options $options): bool {
  // explicitly disabled
  if (is_file("$test.noserver") ||
      (is_file("$test.nowebserver") && $options->server)) {
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

const int SERVER_TIMEOUT = 45;

function run_config_server(Options $options, string $test): mixed {
  invariant(
    can_run_server_test($test, $options),
    'should_skip_test_simple should have skipped this',
  );

  Status::createTestTmpDir($test); // force it to be created
  $config = find_file_for_dir(dirname($test), 'config.ini') ?? '';
  $servers = $options->servers as Servers;
  $port = $servers->configs[$config]->port;
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

  return run_config_post(tuple($output, '', 0), $test, $options);
}

function run_config_cli(
  Options $options,
  string $test,
  string $cmd,
  dict<string, mixed> $cmd_env,
): ?(string, string, int) {
  $cmd = timeout_prefix() . $cmd;

  if ($options->repo && $options->repo_out is null) {
    // we already created it in run_test
    $cmd_env['HPHP_TEST_TMPDIR'] = Status::getTestTmpPath($test, 'tmpdir');
  } else {
    $cmd_env['HPHP_TEST_TMPDIR'] = Status::createTestTmpDir($test);
  }
  $cmd_env['HPHP_TEST_SOURCE_FILE'] = $test;
  if ($options->log) {
    $cmd_env['TRACE'] = 'printir:1';
    $cmd_env['HPHP_TRACE_FILE'] = $test . '.log';
  }

  if ($options->retranslate_all is nonnull ||
      $options->recycle_tc is nonnull ||
      $options->cli_server) {
    $cmd_env['HHVM_MULTI_COUNT_SEP'] = MULTI_REQUEST_SEP;
  }

  $descriptorspec = dict[
    0 => vec["pipe", "r"],
    1 => vec["pipe", "w"],
    2 => vec["pipe", "w"],
  ];
  $pipes = null;
  $process = proc_open(
    "$cmd 2>&1", $descriptorspec, inout $pipes, null, $cmd_env
  );
  if (!is_resource($process)) {
    Status::writeDiff($test, "Couldn't invoke $cmd");
    return null;
  }

  $pipes as nonnull;
  fclose($pipes[0]);
  $output = stream_get_contents($pipes[1]);
  $stderr = stream_get_contents($pipes[2]);
  fclose($pipes[1]);
  fclose($pipes[2]);
  $exit_code = proc_close($process);

  return tuple($output, $stderr, $exit_code);
}

function replace_object_resource_ids(string $str, string $replacement): string {
  $str = preg_replace(
    '/(object\([^)]+\)#)\d+/', '\1'.$replacement, $str
  );
  return preg_replace(
    '/resource\(\d+\)/', "resource($replacement)", $str
  );
}

// Attempt to extract useful crash information from stdout. Ideally we
// could just print stderr, but a test might output text to stderr (even
// when the test is working). When we run the test, we direct stderr
// to stdout anyways. So, any crash information is printed to stdout.
function trim_error_stdout(string $str): string {
  $trimmed = vec[];
  $found_dump = false;
  // Any of the below strings indicate the beginning of a crash
  // report, so only return anything starting with that.
  foreach (explode("\n", $str) as $line) {
    if (!$found_dump) {
      if (stripos($line, "Core dumped") === 0 ||
          stripos($line, "Stack trace in") === 0 ||
          stripos($line, "Assertion Failure") !== false) {
        $found_dump = true;
      }
    }
    if ($found_dump) $trimmed[] = $line;
  }
  if ($found_dump) return implode("\n", $trimmed);
  // If we didn't find anything that looked like a crash report, just
  // supply the last 100. Hopefully there's something there that's
  // useful.
  return implode("\n", array_slice(explode("\n", $str), -100));
}

function run_config_post(
  (string, string, int) $outputs,
  string $test,
  Options $options,
): mixed {

  list($file, $type) = get_expect_file_and_type($test, $options);
  if ($file is null || $type is null) {
    Status::writeDiff(
      $test,
      "No $test.expect or $test.expectf. " .
      "If $test is meant to be included by other tests, " .
      "use a different file extension.\n"
    );
    return false;
  }

  $check_hhbbc_error = $options->repo
    && (file_exists($test . '.hhbbc_assert') ||
        file_exists($test . '.hphpc_assert'));

  $repeats = 0;
  if (!$check_hhbbc_error) {
    if ($options->retranslate_all is nonnull) {
      $repeats = (int)$options->retranslate_all * 2;
    }

    if ($options->recycle_tc is nonnull) {
      $repeats = (int)$options->recycle_tc;
    }

    if ($options->cli_server) {
      $repeats = 3;
    }
  }

  list($output, $stderr, $exit_code) = $outputs;
  if (!$repeats) {
    $split = vec[trim($output)];
  } else {
    $split = array_map(trim<>, explode(MULTI_REQUEST_SEP, $output));
  }

  $output = str_replace(MULTI_REQUEST_SEP, '', $output);
  file_put_contents(Status::getTestOutputPath($test, 'out'), $output);

  // Exit code is 1 for things like fatal errors, which aren't actual
  // crashes.
  if (!$check_hhbbc_error) {
    $exit_code_file = $test . '.exit_code';
    if (file_exists($exit_code_file)) {
      $ok = ((int)trim(file_get_contents($exit_code_file)) === $exit_code);
    } else {
      $ok = ($exit_code === 0 || $exit_code === 1);
    }

    if (!$ok) {
      $trimmed = trim_error_stdout($output);
      Status::writeDiff(
        $test,
        "Test failed because the process returned exit code $exit_code" .
        (($exit_code === 124) ? " (timed out)" : "") .
        "\nstderr:\n$stderr\nstdout:\n$trimmed"
      );
      return false;
    }
  }

  // hhvm redirects errors to stdout, so anything on stderr is really bad.
  if ($stderr && !$check_hhbbc_error) {
    $trimmed = trim_error_stdout($output);
    Status::writeDiff(
      $test,
      "Test failed because the process wrote on stderr:" .
      "\n$stderr\nstdout:\n$trimmed"
    );
    return false;
  }

  if ($repeats > 0 && count($split) != $repeats) {
    Status::writeDiff(
      $test,
      count($split) . ' sets of output returned, expected ' . $repeats
    );
    return false;
  }

  $wanted = trim(file_get_contents($file));

  if ($type === 'expect') {
    if ($options->ignore_oids || $options->repo) {
      $split = array_map($s ==> replace_object_resource_ids($s, 'n'), $split);
      $wanted = replace_object_resource_ids($wanted, 'n');
    }

    foreach ($split as $s) {
      if (strcmp($s, $wanted)) {
        Status::writeDiff(
          $test,
          generate_diff($wanted, $s, ($l1, $l2) ==> !strcmp($l1, $l2))
        );
        return false;
      }
    }
    return true;
  } else if ($type === 'expectf') {
    if ($options->ignore_oids || $options->repo) {
      $wanted = replace_object_resource_ids($wanted, '%d');
    }

    try {
      foreach ($split as $s) {
        $parser = new ExpectfParser($s, $wanted);
        if (!$parser->foundWildcard()) {
          Status::writeDiff(
            $test,
            'Bad expectf file: File contains no actual wildcards. ' .
            'Use expect instead'
          );
          return false;
        }
        if (!$parser->succeeded()) {
          $diff = generate_diff(
            $wanted,
            $s,
            ($l1, $l2) ==> (new ExpectfParser($l2, $l1))->succeeded()
          );
          Status::writeDiff($test, $diff);
          return false;
        }
      }
      return true;
    } catch (BadExpectfPattern $e) {
      Status::writeDiff($test, 'Bad expectf file: ' . $e->getMessage());
      return false;
    }
  } else {
    throw new Exception("Unsupported expect file type: ".$type);
  }
}

// Not all versions of /usr/bin/timeout support the -v option. Test to
// see if it does.
<<__Memoize>>
function timeout_supports_verbose(): string {
  $output = null;
  $exit_code = -1;
  exec(
    '/usr/bin/timeout -v 1000 /usr/bin/true 2> /dev/null',
    inout $output,
    inout $exit_code
  );
  if ($exit_code === 0) return ' -v';
  return '';
}

function timeout_prefix(): string {
  if (is_executable('/usr/bin/timeout')) {
    return
      '/usr/bin/timeout' . timeout_supports_verbose() . ' ' .
      TIMEOUT_SECONDS . ' ';
  } else {
    return hphp_home() . '/hphp/tools/timeout.sh -t ' . TIMEOUT_SECONDS . ' ';
  }
}

function run_foreach_config(
  Options $options,
  string $test,
  vec<string> $cmds,
  dict<string, mixed> $cmd_env,
): mixed {
  invariant(count($cmds) > 0, "run_foreach_config: no modes");
  $result = false;
  foreach ($cmds as $cmd) {
    $outputs = run_config_cli($options, $test, $cmd, $cmd_env);
    if ($outputs is null) return false;
    $result = run_config_post($outputs, $test, $options);
    if (!$result) return $result;
  }
  return $result;
}

function run_and_log_test(Options $options, string $test): void {
  $stime = time();
  $time = mtime();
  $status = run_test($options, $test);
  $time = mtime() - $time;
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
      "invalid skip status %s",
      $status
    );
    Status::skip($test, substr($status, 5), $time, $stime, $etime);
    clean_intermediate_files($test, $options);
  } else {
    invariant_violation("invalid status type %s", gettype($status));
  }
}

// Returns "(string | bool)".
function run_test(Options $options, string $test): mixed {
  $skip_reason = should_skip_test_simple($options, $test);
  if ($skip_reason is nonnull) return $skip_reason;

  if (!$options->no_skipif) {
    $result = runif_should_skip_test($options, $test);
    if (!$result['valid']) {
      invariant(Shapes::keyExists($result, 'error'), 'missing runif error');
      Status::writeDiff($test, 'Invalid .runif file: ' . $result['error']);
      return false;
    }
    if (!($result['match'] ?? false)) {
      invariant(Shapes::keyExists($result, 'skip_reason'), 'missing skip_reason');
      return $result['skip_reason'];
    }

    $result = skipif_should_skip_test($options, $test);
    if (!$result['valid']) {
      invariant(Shapes::keyExists($result, 'error'), 'missing skipif error');
      Status::writeDiff($test, $result['error']);
      return false;
    }
    if (!($result['match'] ?? false)) {
      invariant(Shapes::keyExists($result, 'skip_reason'), 'missing skip_reason');
      return $result['skip_reason'];
    }
  }

  list($hhvm, $hhvm_env) = hhvm_cmd($options, $test);

  if (preg_grep('/ --count[ =][0-9]+ .* --count[ =][0-9]+( |$)/', $hhvm)) {
    // we got --count from 2 sources (e.g. .opts file and multi_request_mode)
    // this can't work so skip the test
    return 'skip-count';
  } else if ($options->jit_serialize is nonnull) {
    // jit-serialize adds the --count option later, so even 1 --count in the
    // command means we have to skip
    if (preg_grep('/ --count[ =][0-9]+( |$)/', $hhvm)) {
      return 'skip-count';
    }
  }

  if ($options->repo) {
    return run_repo_test($options, $test, $hhvm, $hhvm_env);
  }

  if ($options->hhas_round_trip) {
    invariant(
      substr($test, -5) !== ".hhas",
      'should_skip_test_simple should have skipped this',
    );
    // create tmpdir now so that we can write hhas
    Status::createTestTmpDir($test);
    // dumping hhas, not running code so arbitrarily picking a mode
    $hhas_temp = dump_hhas_to_temp($hhvm[0], $test);
    if ($hhas_temp is null) {
      $err = "system failed: " .
        dump_hhas_cmd($hhvm[0], $test,
          Status::getTestTmpPath($test, 'round_trip.hhas')) .
        "\n";
      Status::writeDiff($test, $err);
      return false;
    }
    list($hhvm, $hhvm_env) = hhvm_cmd($options, $test, $hhas_temp);
  }

  if ($options->server) {
    return run_config_server($options, $test);
  }
  return run_foreach_config($options, $test, $hhvm, $hhvm_env);
}

// Returns "(string | bool)".
function run_repo_test(
  Options $options,
  string $test,
  vec<string> $hhvm,
  dict<string, mixed> $hhvm_env,
): mixed {
  $test_repo = test_repo($options, $test);
  if ($options->repo_out is nonnull) {
    // we may need to clean up after a previous run
    $repo_files = vec['hhvm.hhbc', 'hhvm.hhbbc', 'file.cache'];
    foreach ($repo_files as $repo_file) {
      @unlink("$test_repo/$repo_file");
    }
  } else {
    // create tmpdir now so that we can write repos
    Status::createTestTmpDir($test);
  }

  if (file_exists($test . '.hphpc_assert')) {
    $hphp = hphp_cmd($options, $test);
    return run_foreach_config($options, $test, vec[$hphp], $hhvm_env);
  } else if (file_exists($test . '.hhbbc_assert')) {
    $hphp = hphp_cmd($options, $test);
    if (repo_separate($options, $test)) {
      $result = exec_with_stack($hphp);
      if ($result is string) return false;
      $hhbbc = hhbbc_cmd($options, $test);
      return run_foreach_config($options, $test, vec[$hhbbc], $hhvm_env);
    } else {
      return run_foreach_config($options, $test, vec[$hphp], $hhvm_env);
    }
  }

  if (!repo_mode_compile($options, $test)) {
    return false;
  }

  if ($options->hhbbc2) {
    invariant(
      count($hhvm) === 1,
      "get_options forbids modes because we're not running code"
    );
    // create tmpdir now so that we can write hhas
    Status::createTestTmpDir($test);
    $hhas_temp1 = dump_hhas_to_temp($hhvm[0], "$test.before");
    if ($hhas_temp1 is null) {
      Status::writeDiff($test, "dumping hhas after first hhbbc pass failed");
      return false;
    }
    shell_exec("mv $test_repo/hhvm.hhbbc $test_repo/hhvm.hhbc");
    $hhbbc = hhbbc_cmd($options, $test);
    $result = exec_with_stack($hhbbc);
    if ($result is string) {
      Status::writeDiff($test, $result);
      return false;
    }
    $hhas_temp2 = dump_hhas_to_temp($hhvm[0], "$test.after");
    if ($hhas_temp2 is null) {
      Status::writeDiff($test, "dumping hhas after second hhbbc pass failed");
      return false;
    }
    $diff = shell_exec("diff $hhas_temp1 $hhas_temp2");
    if (trim($diff) !== '') {
      Status::writeDiff($test, $diff);
      return false;
    }
  }

  if ($options->jit_serialize is nonnull) {
    invariant(count($hhvm) === 1, 'get_options enforces jit mode only');
    $cmd = jit_serialize_option($hhvm[0], $test, $options, true);
    $outputs = run_config_cli($options, $test, $cmd, $hhvm_env);
    if ($outputs is null) return false;
    $cmd = jit_serialize_option($hhvm[0], $test, $options, true);
    $outputs = run_config_cli($options, $test, $cmd, $hhvm_env);
    if ($outputs is null) return false;
    $hhvm[0] = jit_serialize_option($hhvm[0], $test, $options, false);
  }

  return run_foreach_config($options, $test, $hhvm, $hhvm_env);
}

function num_cpus(): int {
  switch (PHP_OS) {
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
      return (int)exec('sysctl -n hw.ncpu', inout $output, inout $return_var);
  }
  return 2; // default when we don't know how to detect.
}

function make_header(string $str): string {
  return "\n\033[0;33m".$str."\033[0m\n";
}

function print_commands(
  vec<string> $tests,
  Options $options,
): void {
  if (C\count($tests) === 0) {
    print make_header(
      "Test run failed with no failed tests; did a worker process die?"
    );
  } else if ($options->verbose) {
    print make_header("Run these by hand:");
  } else {
    $test = $tests[0];
    print make_header("Run $test by hand:");
    $tests = vec[$test];
  }

  foreach ($tests as $test) {
    list($commands, $_) = hhvm_cmd($options, $test);
    if (!$options->repo) {
      foreach ($commands as $c) {
        print "$c\n";
      }
      continue;
    }

    // How to run it with hhbbc:
    $hhbbc_cmds = hphp_cmd($options, $test)."\n";
    if (repo_separate($options, $test)) {
      $hhbbc_cmd  = hhbbc_cmd($options, $test)."\n";
      $hhbbc_cmds .= $hhbbc_cmd;
      if ($options->hhbbc2) {
        foreach ($commands as $c) {
          $hhbbc_cmds .=
            $c." -vEval.DumpHhas=1 > $test.before.round_trip.hhas\n";
        }
        $test_repo = test_repo($options, $test);
        $hhbbc_cmds .=
          "mv $test_repo/hhvm.hhbbc $test_repo/hhvm.hhbc\n";
        $hhbbc_cmds .= $hhbbc_cmd;
        foreach ($commands as $c) {
          $hhbbc_cmds .=
            $c." -vEval.DumpHhas=1 > $test.after.round_trip.hhas\n";
        }
        $hhbbc_cmds .=
          "diff $test.before.round_trip.hhas $test.after.round_trip.hhas\n";
      }
    }
    if ($options->jit_serialize is nonnull) {
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
function msg_loop(int $num_tests, Queue $queue): void {
  $cols = null;
  $do_progress =
    $num_tests > 0 &&
    (
      Status::getMode() === Status::MODE_NORMAL ||
      Status::getMode() === Status::MODE_RECORD_FAILURES
    ) &&
    Status::hasCursorControl();
  if ($do_progress) {
    $stty = strtolower(Status::getSTTY());
    $matches = vec[];
    if (preg_match_with_matches("/columns ([0-9]+);/", $stty, inout $matches) ||
        // because BSD has to be different
        preg_match_with_matches("/([0-9]+) columns;/", $stty, inout $matches)) {
      $cols = (int)$matches[1];
    }
  }

  while (true) {
    list($pid, $type, $message) = $queue->receiveMessage();
    if (!Status::handle_message($type, $message)) break;

    if ($cols is nonnull) {
      $total_run = (Status::$skipped + Status::$failed + Status::$passed);
      $bar_cols = $cols - 45;

      $passed_ticks  = (int)round($bar_cols * (Status::$passed / $num_tests));
      $skipped_ticks = (int)round($bar_cols * (Status::$skipped / $num_tests));
      $failed_ticks  = (int)round($bar_cols * (Status::$failed / $num_tests));

      $fill = $bar_cols - ($passed_ticks + $skipped_ticks + $failed_ticks);
      if ($fill < 0) $fill = 0;

      $passed_ticks = str_repeat('#', $passed_ticks);
      $skipped_ticks = str_repeat('#', $skipped_ticks);
      $failed_ticks = str_repeat('#', $failed_ticks);
      $fill = str_repeat('-', (int)$fill);

      echo
        "\033[2K\033[1G[",
        "\033[0;32m$passed_ticks",
        "\033[33m$skipped_ticks",
        "\033[31m$failed_ticks",
        "\033[0m$fill] ($total_run/$num_tests) ",
        "(", Status::$skipped, " skipped,", Status::$failed, " failed)";
    }
  }

  if ($cols is nonnull) {
    print "\033[2K\033[1G";
    if (Status::$skipped > 0) {
      print Status::$skipped ." tests \033[1;33mskipped\033[0m\n";
      $reasons = Status::$skip_reasons;
      arsort(inout $reasons);
      Status::$skip_reasons = $reasons as dict<_, _>;
      foreach (Status::$skip_reasons as $reason => $count) {
        printf("%12s: %d\n", $reason, $count);
      }
    }
  }
}

function print_success(
  vec<string> $tests,
  dict<string, TestResult> $results,
  Options $options,
): void {
  // We didn't run any tests, not even skipped. Clowntown!
  if (!$tests) {
    print "\nCLOWNTOWN: No tests!\n";
    if (!$options->no_fun) {
      print_clown();
    }
    return;
  }
  $ran_tests = false;
  foreach ($results as $result) {
    // The result here will either be skipped or passed (since failed is
    // handled in print_failure.
    if ($result['status'] === 'passed') {
      $ran_tests = true;
      break;
    }
  }
  // We just had skipped tests
  if (!$ran_tests) {
    print "\nSKIP-ALOO: Only skipped tests!\n";
    if (!$options->no_fun) {
      print_skipper();
    }
    return;
  }
  print "\nAll tests passed.\n";
  if (!$options->no_fun) {
    print_ship();
  }
  if ($options->failure_file is nonnull) {
    @unlink($options->failure_file);
  }
  if ($options->verbose) {
    print_commands($tests, $options);
  }
}

function print_failure(
  vec<string> $argv,
  dict<string, TestResult> $results,
  Options $options,
): void {
  $failed = vec[];
  $passed = vec[];
  foreach ($results as $result) {
    if ($result['status'] === 'failed') {
      $failed[] = $result['name'];
    } else if ($result['status'] === 'passed') {
      $passed[] = $result['name'];
    }
  }
  sort(inout $failed);

  $failing_tests_file = $options->failure_file ??
    Status::getRunTmpDir() . '/test-failures';
  file_put_contents($failing_tests_file, implode("\n", $failed)."\n");
  if ($passed) {
    $passing_tests_file = Status::getRunTmpDir() . '/tests-passed';
    file_put_contents($passing_tests_file, implode("\n", $passed)."\n");
  } else {
    $passing_tests_file = "";
  }

  print "\n".count($failed)." tests failed\n";
  if (!$options->no_fun) {
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
    if ($n === 2 && !$options->verbose) {
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

function port_is_listening(int $port): bool {
  $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
  return @socket_connect($socket, 'localhost', $port);
}

function find_open_port(): int {
  for ($i = 0; $i < 50; ++$i) {
    $port = rand(1024, 65535);
    if (!port_is_listening($port)) return $port;
  }

  error("Couldn't find an open port");
}

function start_server_proc(
  Options $options,
  string $config,
  int $port,
): Server {
  if ($options->cli_server) {
    $cli_sock = tempnam(sys_get_temp_dir(), 'hhvm-cli-');
  } else {
    // still want to test that an unwritable socket works...
    $cli_sock = '/var/run/hhvm-cli.sock';
  }
  $threads = get_num_threads($options);
  $thread_option = $options->cli_server
    ? '-vEval.UnixServerWorkers='.$threads
    : '-vServer.ThreadCount='.$threads;
  $prelude = $options->server
    ? '-vEval.PreludePath=' . Status::getRunTmpDir() . '/server-prelude.php'
    : "";
  $command = hhvm_cmd_impl(
    $options,
    $config,
    // Provide a temporary file for `Autoload.DB.Path`: without this, we'll fail
    // to run the server with native autoloading.
    tempnam(Status::getRunTmpDir(), 'autoloadDB_%{schema}_'),
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
    '-vEval.JitASize=394264576',
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

  $descriptors = dict[
    0 => vec['file', '/dev/null', 'r'],
    1 => vec['file', '/dev/null', 'w'],
    2 => vec['file', '/dev/null', 'w'],
  ];

  $dummy = null;
  $proc = proc_open($command, $descriptors, inout $dummy);
  if (!$proc) {
    error("Failed to start server process");
  }
  $status = proc_get_status($proc); // dict<string, mixed>
  $pid = $status['pid'] as int;
  $server = new Server($proc, $pid, $port, $config, $cli_sock);
  return $server;
}

final class Server {
  public function __construct(
    public resource $proc,
    public int $pid,
    public int $port,
    public string $config,
    public string $cli_socket,
  ) {
  }
}

final class Servers {
  public dict<int, Server> $pids = dict[];
  public dict<string, Server> $configs = dict[];
}

// For each config file in $configs, start up a server on a randomly-determined
// port.
function start_servers(
  Options $options,
  keyset<string> $configs,
): Servers {
  if ($options->server) {
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

  $starting = vec[];
  foreach ($configs as $config) {
    $starting[] = start_server_proc($options, $config, find_open_port());
  }

  $start_time = mtime();
  $servers = new Servers();

  // Wait for all servers to come up.
  while (count($starting) > 0) {
    $still_starting = vec[];

    foreach ($starting as $server) {
      $config = $server->config;
      $pid = $server->pid;
      $port = $server->port;
      $proc = $server->proc;

      $new_status = proc_get_status($proc);

      if (!$new_status['running']) {
        if ($new_status['exitcode'] === 0) {
          error("Server exited prematurely but without error");
        }

        // We lost a race. Try another port.
        if (getenv('HHVM_TEST_SERVER_LOG')) {
          echo "\n\nLost connection race on port $port. Trying another.\n\n";
        }
        $port = find_open_port();
        $still_starting[] = start_server_proc($options, $config, $port);
      } else if (!port_is_listening($port)) {
        $still_starting[] = $server;
      } else {
        $servers->pids[$pid] = $server;
        $servers->configs[$config] = $server;
      }
    }

    $starting = $still_starting;
    $max_time = 30;
    if (mtime() - $start_time > $max_time) {
      error("Servers took more than $max_time seconds to come up");
    }

    // Take a short nap and try again.
    usleep(100000);
  }

  $elapsed = mtime() - $start_time;
  printf("Started %d servers in %.1f seconds\n\n", count($configs), $elapsed);
  return $servers;
}

function get_num_threads(Options $options): int {
  if ($options->threads is nonnull) {
    $threads = (int)$options->threads;
    if ((string)$threads !== $options->threads || $threads < 1) {
      error("--threads must be an integer >= 1");
    }
  } else {
    $threads = $options->server || $options->cli_server
      ? num_cpus() * 2 : num_cpus();
  }
  return $threads;
}

function runner_precheck(): void {
  // Basic checking for runner.
  $server = HH\global_get('_SERVER');
  $env = HH\global_get('_ENV');
  if (!((bool)$server ?? false) || !((bool)$env ?? false)) {
    echo "Warning: \$_SERVER/\$_ENV variables not available, please check \n" .
         "your ini setting: variables_order, it should have both 'E' and 'S'\n";
  }
}

function main(vec<string> $argv): int {
  runner_precheck();

  ini_set('pcre.backtrack_limit', PHP_INT_MAX);

  list($options, $files) = get_options($argv);
  if ($options->help) {
    error(help());
  }

  Status::createTmpDir($options->working_dir);

  if ($options->list_tests) {
    list_tests($files, $options);
    print "\n";
    exit(0);
  }

  $tests = find_tests($files, $options);
  if ($options->shuffle) {
    shuffle(inout $tests);
  }

  // Explicit path given by --hhvm-binary-path takes priority. Then, if an
  // HHVM_BIN env var exists, and the file it points to exists, that trumps
  // any default hhvm executable path.
  if ($options->hhvm_binary_path is nonnull) {
    $binary_path = check_executable($options->hhvm_binary_path);
    putenv("HHVM_BIN=" . $binary_path);
  } else if (getenv("HHVM_BIN") !== false) {
    $binary_path = check_executable(getenv("HHVM_BIN"));
  } else {
    check_for_multiple_default_binaries();
    $binary_path = hhvm_path();
  }

  if ($options->verbose) {
    print "You are using the binary located at: " . $binary_path . "\n";
  }

  $servers = null;
  if ($options->server || $options->cli_server) {
    if ($options->server && $options->cli_server) {
      error("Server mode and CLI Server mode are mutually exclusive");
    }
    if ($options->repo) {
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
      if (should_skip_test_simple($options, $test) is nonnull) continue;
      $configs[] = $config;
    }

    $max_configs = 10;
    if (count($configs) > $max_configs) {
      error("More than $max_configs unique config files will be needed to run ".
            "the tests you specified. They may not be a good fit for server ".
            "mode. (".count($configs)." required)");
    }

    $servers = start_servers($options, $configs);
    $options->servers = $servers;
  }

  // Try to construct the buckets so the test results are ready in
  // approximately alphabetical order.
  // Get the serial tests to be in their own bucket later.
  $serial_tests = serial_only_tests($tests);

  // If we have no serial tests, we can use the maximum number of allowed
  // threads for the test running. If we have some, we save one thread for
  // the serial bucket. However if we only have one thread, we don't split
  // out serial tests.
  $parallel_threads = min(get_num_threads($options), \count($tests)) as int;
  if ($parallel_threads === 1) {
    $test_buckets = vec[$tests];
  } else {
    if (count($serial_tests) > 0) {
      // reserve a thread for serial tests
      $parallel_threads--;
    }

    $test_buckets = vec[];
    for ($i = 0; $i < $parallel_threads; $i++) {
      $test_buckets[] = vec[];
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
  if (!$options->testpilot) {
    print "Running ".count($tests)." tests in ".
      count($test_buckets)." threads (" . count($serial_tests) .
      " in serial)\n";
  }

  if ($options->verbose) {
    Status::setMode(Status::MODE_VERBOSE);
  }
  if ($options->testpilot) {
    Status::setMode(Status::MODE_TESTPILOT);
  }
  if ($options->record_failures is nonnull) {
    Status::setMode(Status::MODE_RECORD_FAILURES);
  }
  Status::setUseColor($options->color || posix_isatty(STDOUT));

  Status::$nofork = count($tests) === 1 && !$servers;

  if (!Status::$nofork) {
    // Create the Queue before any children are forked.
    $queue = Status::getQueue();

    // Fork a "printer" child to process status messages.
    $printer_pid = pcntl_fork();
    if ($printer_pid === -1) {
      error("failed to fork");
    } else if ($printer_pid === 0) {
      msg_loop(count($tests), $queue);
      return 0;
    }
  } else {
    // Satisfy the type-checker.
    $printer_pid = -1;
  }

  // Unblock the Queue (if needed).
  Status::started();

  // Fork "worker" children (if needed).
  $children = dict[];
  // We write results as json in each child and collate them at the end
  $json_results_files = vec[];
  if (Status::$nofork) {
    Status::registerCleanup($options->no_clean);
    $json_results_file = tempnam('/tmp', 'test-run-');
    $json_results_files[] = $json_results_file;
    invariant(count($test_buckets) === 1, "nofork was set erroneously");
    $return_value = child_main($options, $test_buckets[0], $json_results_file);
  } else {
    foreach ($test_buckets as $test_bucket) {
      $json_results_file = tempnam('/tmp', 'test-run-');
      $json_results_files[] = $json_results_file;
      $pid = pcntl_fork();
      if ($pid === -1) {
        error('could not fork');
      } else if ($pid) {
        $children[$pid] = $pid;
      } else {
        invariant($test_bucket is vec<_>, "%s", __METHOD__);
        exit(child_main($options, $test_bucket, $json_results_file));
      }
    }

    // Make sure to clean up on exit, or on SIGTERM/SIGINT.
    // Do this here so no children inherit this.
    Status::registerCleanup($options->no_clean);

    // Have the parent wait for all forked children to exit.
    $return_value = 0;
    while (count($children) && $printer_pid !== 0) {
      $status = null;
      $pid = pcntl_wait(inout $status);
      if (pcntl_wifexited($status as nonnull)) {
        $bad_end = pcntl_wexitstatus($status) !== 0;
      } else if (pcntl_wifsignaled($status)) {
        $bad_end = true;
      } else {
        error("Unexpected exit status from child");
      }

      if ($pid === $printer_pid) {
        // We should be finishing up soon.
        $printer_pid = 0;
        if ($bad_end) {
          // Don't consider the run successful if the printer worker died
          $return_value = 1;
        }
      } else if ($servers && isset($servers->pids[$pid])) {
        // A server crashed. Restart it.
        // We intentionally ignore $bad_end here because we expect this to
        // show up as a test failure in whatever test was running on the server
        // when it crashed. TODO(alexeyt): assert $bad_end === true?
        if (getenv('HHVM_TEST_SERVER_LOG')) {
          echo "\nServer $pid crashed. Restarting.\n";
        }
        Status::serverRestarted();
        $server = $servers->pids[$pid];
        $server = start_server_proc($options, $server->config, $server->port);

        // Unset the old $pid entry and insert the new one.
        unset($servers->pids[$pid]);
        $pid = $server->pid;
        $servers->pids[$pid] = $server;
      } else if (isset($children[$pid])) {
        unset($children[$pid]);
        if ($bad_end) {
          // If any worker process dies we should fail the test run
          $return_value = 1;
        }
      } else {
        error("Got status for child that we didn't know we had with pid $pid");
      }
    }
  }

  Status::finished($return_value);

  // Wait for the printer child to exit, if needed.
  if (!Status::$nofork && $printer_pid !== 0) {
    $status = 0;
    $pid = pcntl_waitpid($printer_pid, inout $status);
    $status = $status as int;
    if (pcntl_wifexited($status)) {
      if (pcntl_wexitstatus($status) !== 0) {
        // Don't consider the run successful if the printer worker died
        $return_value = 1;
      }
    } else if (pcntl_wifsignaled($status)) {
      // Don't consider the run successful if the printer worker died
      $return_value = 1;
    } else {
      error("Unexpected exit status from child");
    }
  }

  // Kill the servers.
  if ($servers) {
    foreach ($servers->pids as $server) {
      proc_terminate($server->proc);
      proc_close($server->proc);
    }
  }

  // Aggregate results.
  $results = dict[];
  foreach ($json_results_files as $json_results_file) {
    $contents = file_get_contents($json_results_file);
    $json = json_decode($contents, true);
    if (!is_dict($json)) {
      error(
        "\nNo JSON output was received from a test thread. ".
        "Either you killed it, or it might be a bug in the test script.",
      );
    }
    $results = array_merge($results, $json);
    unlink($json_results_file);
  }

  // Print results.
  if ($options->record_failures is nonnull) {
    $fail_file = $options->record_failures;
    $failed_tests = vec[];
    $prev_failing = vec[];
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
  } else if ($options->testpilot) {
    Status::say(dict['op' => 'all_done', 'results' => $results]);
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
  exit(main(get_argv()));
}

function help(): string {
  $argv = get_argv();
  $ztestexample = 'test/zend/good/*/*z*.php'; // sep. for syntax highlighting.
  $help = <<<EOT


This is the hhvm test-suite runner.  For more detailed documentation,
see hphp/test/README.md.

The test argument may be a path to a php test file, a directory name, or
one of a few pre-defined suite names that this script knows about.

If you work with hhvm a lot, you might consider a bash alias:

   alias htest="path/to/hphp/test/run"

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
  % {$argv[0]} test/quick -r --compiler-args '--log=4'

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

// Inline ASCII art moved to end-of-file to avoid confusing emacs.

function print_clown(): void {
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

function print_skipper(): void {
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

function print_ship(): void {
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
