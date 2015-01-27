<?hh
require_once __DIR__.'/SortedIterator.php';
require_once __DIR__.'/Options.php';

class TimeoutException extends Exception {
}

# There's an outer timeout of 300s; this number must be less than that (with
# fudge factor)
const INSTALL_TIMEOUT_SECS = 240;
const NETWORK_RETRIES = 1;

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

  foreach ($files as $fileinfo) {
    if ($fileinfo->isDir()) {
      rmdir($fileinfo->getPathname());
    } else {
      unlink($fileinfo->getPathname());
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
function find_first_file_recursive(Set $filenames, string $root_dir,
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
                        string $exclude_file_pattern,
                        ?Set<string> $exclude_dirs = null): ?Set<string> {
  if (!file_exists($root_dir)) {
    return null;
  }
  $files = Set {};
  $dit = new RecursiveDirectoryIterator($root_dir,
                                        RecursiveDirectoryIterator::SKIP_DOTS);
  $rit = new RecursiveIteratorIterator($dit);
  $sit = new SortedIterator($rit);
  foreach ($sit as $fileinfo) {
    if (preg_match($pattern, $fileinfo->getFileName()) === 1 &&
        preg_match($exclude_file_pattern, $fileinfo->getFileName()) === 0 &&
        strstr($fileinfo->getPath(), '/vendor/') === false &&
        !nullthrows($exclude_dirs)->contains(dirname($fileinfo->getPath()))) {
      $files[] = $fileinfo->getPathName();
    }
  }

  return $files;
}

function find_all_files_containing_text(
  string $text,
  string $root_dir,
  string $exclude_file_pattern,
  ?Set<string> $exclude_dirs = null,
): ?Set<string> {
  if (!file_exists($root_dir)) {
    return null;
  }
  $files = Set {};
  $dit = new RecursiveDirectoryIterator($root_dir,
                                        RecursiveDirectoryIterator::SKIP_DOTS);
  $rit = new RecursiveIteratorIterator($dit);
  $sit = new SortedIterator($rit);
  foreach ($sit as $fileinfo) {
    if (strpos(file_get_contents($fileinfo->getPathName()), $text) !== false &&
        preg_match($exclude_file_pattern, $fileinfo->getFileName()) === 0 &&
        strstr($fileinfo->getPath(), '/vendor/') === false &&
        !nullthrows($exclude_dirs)->contains(dirname($fileinfo->getPath()))) {
      $files[] = $fileinfo->getPathName();
    }
  }

  return $files;
}

function command_exists(string $cmd): bool {
    $ret = shell_exec("which $cmd");
    return !empty($ret);
}

/**
 * Print if output format is for humans
 */
function human(string $msg): void {
  if (
    (Options::$output_format === OutputFormat::HUMAN) ||
    (Options::$output_format === OutputFormat::HUMAN_VERBOSE)
  ) {
    print $msg;
  }
}

function fbmake_json(Map<string, mixed> $data) {
  if (Options::$output_format === OutputFormat::FBMAKE) {
    // Yep, really. STDERR. If you put it on STDOUT instead, 'All tests passed.'
    fprintf(STDERR, "%s\n", json_encode($data));
  }
}

function fbmake_test_name(Framework $framework, string $test) {
  return $framework->getName().'/'.$test;
}

function fbmake_result_json(
  Framework $framework,
  string $test,
  string $status
): Map<string, mixed> {
  if (Options::$output_format !== OutputFormat::FBMAKE) {
    return Map { };
  }

  $expected = $framework->getCurrentTestStatuses();
  if ($expected && $expected->containsKey($test)) {
    $expected = $expected[$test];

    if ($expected === $status) {
      return Map {
        'status' => 'passed',
        'details' => 'Matched expected status: '.$status,
      };
    }
    return Map {
      'status' => 'failed',
      'details' => 'Expected '.$expected.', got '.$status,
    };
  }
  return Map {
    'status' => 'failed',
    'details' => 'Unknown test - updated expect file needed?',
  };
}

/**
 * Print output if verbose mode is on. This implies that the output format
 * is human-readable.
 */
function verbose(string $msg): void {
  if (Options::$output_format === OutputFormat::HUMAN_VERBOSE) {
    print $msg;
  }
}

/**
 * Print output if format is human readable, but not not verbose.
 */
function not_verbose(string $msg): void {
  if (Options::$output_format === OutputFormat::HUMAN) {
    print $msg;
  }
}

function remove_color_codes(string $line): string {
  // Get rid of codes like ^[[31;31m that may get output to the results file.
  // 0x1B is the hex code for the escape sequence ^[
  $color_escape_code_pattern = "/\x1B\[([0-9]{1,2}(;[0-9]{1,2})?)?[m|K]/";
  return preg_replace($color_escape_code_pattern, "", $line);
}

/*
  e.g., remove_string_from_text($dir, __DIR__, null);

  /data/users/joelm/fbcode/hphp/test/frameworks/frameworks/pear-core/tests
  /PEAR_Command_Channels/channel-update/test_remotefile.phpt

  becomes

  frameworks/pear-core/tests//PEAR_Command_Channels/
  channel-update/test_remotefile.phpt

*/
function remove_string_from_text(string $text, string $str,
                                 ?string $replace = null): string {
  if (($pos = strpos($text, $str)) !== false) {
    return $replace === null
           ? substr($text, $pos + strlen(__DIR__) + 1)
           : substr_replace($text, $replace, $pos, strlen(__DIR__) + 1);
  }
  return $text;
}

function get_subclasses_of(string $parent): Vector {
  $result = Vector {};
  foreach (get_declared_classes() as $class) {
    if (is_subclass_of($class, $parent)) {
      $result[] = strtolower($class);
    }
  }
  sort($result);
  return $result;
}

function get_runtime_build(bool $use_php = false): string {
  $executable = '';
  $command = '';

  // FIX: Should we try to install a vanilla php binary here instead of
  // relying on user to specify a path? Should we try to determine if php
  // is already installed via a $PATH variable?
  if (Options::$php_path !== null) {
    if (!file_exists(Options::$php_path)) {
      error_and_exit("PHP build does not exists. Are you sure your path is ".
                     "right?");
    }
    $executable = Options::$php_path;
    $command = $executable;
  } else {
    $fbcode_root_dir = __DIR__.'/../../..';
    $oss_root_dir = __DIR__.'/../..';
    // See if we are using an internal development build
    if ((file_exists($fbcode_root_dir."/_bin"))) {
      $executable = $fbcode_root_dir;
      $executable .= $use_php ? "/_bin/hphp/hhvm/php" : "/_bin/hphp/hhvm/hhvm";
    // Maybe we are in OSS land trying this script
    } else if (file_exists($oss_root_dir."/hhvm")) {
      // Pear won't run correctly unless a 'php' executable exists.
      // This may be a Pear thing, a PHPUnit running phpt thing, or
      // or something else. Until we know for sure, let's just create
      // a php symlink to hhvm
      symlink($oss_root_dir."/hhvm/hhvm", $oss_root_dir."/hhvm/php");

      $executable = $oss_root_dir."/hhvm";
      $executable .= $use_php ? "/php" : "/hhvm";
    } else {
      error_and_exit("HHVM build doesn't exist. Did you build yet?");
    }
    $command = $executable;
    if (!$use_php) {
      $repo_loc = tempnam('/tmp', 'framework-test');
      $repo_args = " -v Repo.Local.Mode=-- -v Repo.Central.Path=".$repo_loc;
      $command .= $repo_args.
        " --config ".__DIR__."/php.ini";
    }
  }
  invariant(
    file_exists($executable),
    $executable.' does not exist'
  );
  invariant(
    is_executable($executable),
    $executable.' is not executable'
  );
  return nullthrows($command);
}

function error_and_exit(
  string $message,
  string $fbmake_action = 'skipped',
): void {
  if (Options::$output_format === OutputFormat::FBMAKE) {
    fprintf(
      STDERR,
      "%s\n",
      json_encode(
        [
          'op' => 'test_done',
          'test' => 'framework test setup',
          'status' => $fbmake_action,
          'details' => 'ERROR: '.$message,
        ],
        /* assoc array = */ true,
      )
    );
    exit(0);
  }
  fprintf(STDERR, "ERROR: %s\n", $message);
  exit(1);
}

// Include all PHP files in a directory
function include_all_php($folder){
  foreach (glob("{$folder}/*.php") as $filename) {
    require_once $filename;
  }
}

// This will run processes that will get the test infra dependencies
// (e.g. PHPUnit), frameworks and framework dependencies.
function run_install(
  string $proc,
  string $path,
  ?Map $env = null,
  int $retries = NETWORK_RETRIES
): ?int {
  // We need to output something every once in a while - if we go quiet, fbmake
  // kills us.
  for ($try = 1; $try <= $retries; ++$try) {
    $test_name = $proc.' - attempt '.$try;
    try {
      fbmake_json(Map {'op' => 'start', 'test' => $test_name});
      $result = run_install_impl($proc, $path, $env);
      fbmake_json(
        Map {'op' => 'test_done', 'test' => $test_name, 'status' => 'passed' }
      );
      return $result;
    } catch (TimeoutException $e) {
      verbose((string) $e);
      remove_dir_recursive(nullthrows($path));
      fbmake_json(
        Map {'op' => 'test_done', 'test' => $test_name, 'status' => 'skipped' }
      );
    }
  }

  error_and_exit('Retries exceeded: '.$proc);
  return null; // unrechable, but make the typechecker happy.
}

function run_install_impl(string $proc, string $path, ?Map $env): ?int
{
  verbose("Running: $proc\n");
  $descriptorspec = array(
    0 => array("pipe", "r"),
    1 => array("pipe", "w"),
    2 => array("pipe", "w"),
  );

  $env_arr = null; // $_ENV will passed in by default if this is null
  if ($env !== null) {
    $env_arr = array_merge($_ENV, $env->toArray());
  }
  // If you have this set, it probably points to hhvm objects, not OSS
  // objects. Misses here seem to be a huge slowdown, causing problems with
  // fbmake timeouts.
  if (
    $env_arr !== null &&
    array_key_exists('GIT_ALTERNATE_OBJECT_DIRECTORIES', $env_arr)
  ) {
    unset($env_arr['GIT_ALTERNATE_OBJECT_DIRECTORIES']);
  }

  $pipes = null;
  $process = proc_open($proc, $descriptorspec, $pipes, $path, $env_arr);
  assert($pipes !== null);
  if (is_resource($process)) {
    fclose($pipes[0]);
    $start_time = microtime(true);

    $read = [$pipes[1]];
    $write = [];
    $except = $read;
    $ready = null;
    $done_by = time() + INSTALL_TIMEOUT_SECS;
    while ($done_by > time()) {
      $remaining = $done_by - time();
      $ready = stream_select(
        $read, $write, $except,
        $remaining > 0 ? $remaining : 1
      );
      if ($ready === 0) {
        proc_terminate($process);
        throw new TimeoutException("Hit timeout reading from proc: ".$proc);
      }
      if (feof($pipes[1])) {
        break;
      }
      $block = fread($pipes[1], 8096);
      verbose($block);
      if ((microtime(true) - $start_time) > 1) {
        not_verbose('.');
        $start_time = microtime(true);
      }
    }
    verbose(stream_get_contents($pipes[2]));
    fclose($pipes[1]);
    $ret = proc_close($process);
    verbose("Returned status $ret\n");
    return $ret;
  }
  verbose("Couldn't proc_open: $proc\n");
  return null;
}

function nullthrows<T>(?T $x, ?string $message = null): T {
  if ($x !== null) {
    return $x;
  }
  if ($message === null) {
    $message = 'Unexpected null';
  }
  throw new Exception($message);
}

// Use this instead of unlink to avoid warnings
function delete_file(?string $path): void {
  if ($path !== null && file_exists($path)) {
    unlink($path);
  }
}
