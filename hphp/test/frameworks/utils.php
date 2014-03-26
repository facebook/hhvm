<?hh
require_once __DIR__.'/SortedIterator.php';

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
                        Set $exclude_dirs = null): ?Set {
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
        !$exclude_dirs->contains(dirname($fileinfo->getPath()))) {
      $files[] = $fileinfo->getPathName();
    }
  }

  return $files;
}

function find_all_files_containing_text(string $text,
                                        string $root_dir,
                                        string $exclude_file_pattern,
                                        Set $exclude_dirs = null): ?Set {
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
        strstr($file_info->getPath(), '/vendor/') === false &&
        !$exclude_dirs->contains(dirname($fileinfo->getPath()))) {
      $files[] = $fileinfo->getPathName();
    }
  }

  return $files;
}

function idx(array $array, mixed $key, mixed $default = null): mixed {
  return isset($array[$key]) ? $array[$key] : $default;
}

function command_exists(string $cmd): bool {
    $ret = shell_exec("which $cmd");
    return !empty($ret);
}

function verbose(string $msg, bool $verbose): void {
  if ($verbose) {
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
  $build = "";

  // FIX: Should we try to install a vanilla zend binary here instead of
  // relying on user to specify a path? Should we try to determine if zend
  // is already installed via a $PATH variable?
  if (Options::$zend_path !== null) {
    if (!file_exists(Options::$zend_path)) {
      error_and_exit("Zend build does not exists. Are you sure your path is ".
                     "right?");
    }
    $build = Options::$zend_path;
  } else {
    $fbcode_root_dir = __DIR__.'/../../..';
    $oss_root_dir = __DIR__.'/../..';
    // See if we are using an internal development build
    if ((file_exists($fbcode_root_dir."/_bin"))) {
      $build .= $fbcode_root_dir;
      $build .= $use_php ? "/_bin/hphp/hhvm/php" : "/_bin/hphp/hhvm/hhvm";
    // Maybe we are in OSS land trying this script
    } else if (file_exists($oss_root_dir."/hhvm")) {
      // Pear won't run correctly unless a 'php' executable exists.
      // This may be a Pear thing, a PHPUnit running phpt thing, or
      // or something else. Until we know for sure, let's just create
      // a php symlink to hhvm
      symlink($oss_root_dir."/hhvm/hhvm", $oss_root_dir."/hhvm/php");

      $build .= $oss_root_dir."/hhvm";
      $build .= $use_php ? "/php" : "/hhvm";
    } else {
      error_and_exit("HHVM build doesn't exist. Did you build yet?");
    }
    if (!$use_php) {
      $repo_loc = tempnam('/tmp', 'framework-test');
      $repo_args = " -v Repo.Local.Mode=-- -v Repo.Central.Path=".$repo_loc;
      $build .= $repo_args.
        " --config ".__DIR__."/config.hdf".
        " --config ".__DIR__."/php.ini";
    }
  }
  return $build;
}

function error_and_exit(string $message, bool $to_file = false): void {
  if ($to_file) {
    $target = Options::$script_errors_file;
  } else {
    $target = 'php://stderr';
  }
  file_put_contents($target, basename(__FILE__).": ".
                    $message.PHP_EOL, FILE_APPEND);
  exit(1);
}

// Include all PHP files in a directory
function include_all_php($folder){
  foreach (glob("{$folder}/*.php") as $filename) {
    include_once $filename;
  }
}

// This will run processes that will get the test infra dependencies
// (e.g. PHPUnit), frameworks and framework dependencies.
function run_install(string $proc, string $path, ?Map $env): ?int
{
  verbose("Running: $proc\n", Options::$verbose);
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
    while ($line = fgets($pipes[1])) {
      verbose("$line", Options::$verbose);
      if ((microtime(true) - $start_time) > 1) {
        verbose(".", !Options::$verbose && !Options::$csv_only);
        $start_time = microtime(true);
      }
    }
    verbose(stream_get_contents($pipes[2]), Options::$verbose);
    fclose($pipes[1]);
    $ret = proc_close($process);
    verbose("Returned status $ret\n", Options::$verbose);
    return $ret;
  }
  verbose("Couldn't proc_open: $proc\n", Options::$verbose);
  return null;
}
