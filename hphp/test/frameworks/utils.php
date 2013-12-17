<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

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
    return (empty($ret) ? false : true);
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
