<?hh

/**
 * Try to detect (and forbid) external dependencies.
 *
 * Use this by setting:
 *  - hhvm.jit_enable_rename_function=true
 *  - auto_prepend_file=/path/to/Isolation.php
 */

class IsolationException extends Exception { }

class Isolation {
  private static $allowedDirectories = Set { };
  private static $allowedFiles = Set { };

  public static function AllowDirectory(string $path) {
    self::$allowedDirectories[] = realpath($path);
  }
  public static function AllowFile(string $path) {
    self::$allowedFiles[] = realpath($path);
  }

  public static function CheckURL(string $url) {
    if (strpos($url, 'localhost') === false) {
      throw new IsolationException('Tried to open URL: '.$url);
    }
  }

  public static function CheckReadFile(string $path) {
    if (strpos($path, 'http://') === 0 || strpos($path, 'https://') === 0) {
      CheckURL($path);
      return;
    }
  }

  public static function CheckWriteFile(string $path) {
    $scheme = parse_url($path, PHP_URL_SCHEME);
    if ($scheme === 'vfs') {
      // vfsstream already provides isolation for unit tests, allow it
      return;
    } else if ($scheme === 'php') {
      // memory, temp, etc
      return;
    }
    $realpath = realpath($path);
    if ($realpath === false) {
      throw new IsolationException('Unable to confirm if path is safe: '.$path);
    }
    $path = $realpath;
    if (self::$allowedFiles->contains($path)) {
      return;
    }
    foreach (self::$allowedDirectories as $dir) {
      if (strpos($path, $dir.'/') === 0) {
        return;
      }
    }
    throw new IsolationException('Tried to write to file: '.$path);
  }

  public static function CheckExec($name, $obj, $args, $data, &$done) {
    if (preg_match(',(^|[/"\'])(node|git|hhvm)[ "\'],', $args[0])) {
      // Allow
      $done = false;
      return;
    }
    throw new IsolationException(
      'Tried to '.$name.'() with args:'.var_export($args, true)
    );
  }
}

//////////
// URLs //
//////////


fb_intercept(
  'curl_init',
  function($name, $obj, $args, $data, &$done) {
    if (count($args) > 0) {
      Isolation::CheckURL($args[0]);
    }
    $done = false;
  }
);
fb_intercept(
  'curl_setopt',
  function($name, $obj, $args, $data, &$done) {
    list ($ch, $option, $value) = $args;
    if ($option === CURLOPT_URL) {
      Isolation::CheckURL($value);
    }
    $done = false;
  }
);
fb_intercept(
  'curl_setopt_array',
  function($name, $obj, $args, $data, &$done) {
    list ($ch, $options) = $args;
    foreach ($options as $opt => $value) {
      if ($opt === CURLOPT_URL) {
        Isolation::CheckURL($value);
      }
    }
    $done = false;
  }
);

//////////////////////////
// Files - Whitelisting //
//////////////////////////

function sys_get_temp_dir_ISOLATION_WRAPPER() {
  // Make sure we don't just return '/tmp' or other easily-guessable location.
  static $dir = null;
  if ($dir === null) {
    $dir = ORIG_sys_get_temp_dir();
    $dir .= '/'.uniqid('test_isolation', true);
    mkdir($dir);
  }
  return $dir;
}
fb_rename_function('sys_get_temp_dir', 'ORIG_sys_get_temp_dir');
fb_rename_function('sys_get_temp_dir_ISOLATION_WRAPPER', 'sys_get_temp_dir');
Isolation::AllowDirectory(sys_get_temp_dir());

function tempnam_ISOLATION_WRAPPER() {
  $path = call_user_func_array('ORIG_tempnam', func_get_args());
  Isolation::AllowFile($path);
  return $path;
}
fb_rename_function('tempnam', 'ORIG_tempnam');
fb_rename_function('tempnam_ISOLATION_WRAPPER', 'tempnam');

function time_ISOLATION_WRAPPER() {
  static $time;
  $org_time = ORIG_time();
  if ($time === null) {
    $time = $org_time;
    return $time;
  }
  if ($time >= $org_time) {
    ++$time;
  } else {
    $time = $org_time;
  }
  return $time;
}
fb_rename_function('time', 'ORIG_time');
fb_rename_function('time_ISOLATION_WRAPPER', 'time');

function microtime_ISOLATION_WRAPPER(bool $get_as_float = false) {
  static $time;
  list($msec, $sec) = explode(" ", ORIG_microtime());
  if ($time === null) {
    $time = $sec;
  } else if ($time >= $sec){
    ++$time;
  } else {
    $time = $sec;
  }
  if ($get_as_float)
    return ((float)$msec + (float)$time);
  else
    return "{$msec} {$time}";
}
fb_rename_function('microtime', 'ORIG_microtime');
fb_rename_function('microtime_ISOLATION_WRAPPER', 'microtime');

function date_ISOLATION_WRAPPER(?string $format, ?int $timestamp = null) {
  $timestamp = ($timestamp === null ? time() : $timestamp);
  return ORIG_date($format, $timestamp);
}
fb_rename_function('date', 'ORIG_date');
fb_rename_function('date_ISOLATION_WRAPPER', 'date');

function gmdate_ISOLATION_WRAPPER(?string $format, ?int $timestamp = null) {
  $timestamp = ($timestamp === null ? time() : $timestamp);
  return ORIG_gmdate($format, $timestamp);
}
fb_rename_function('gmdate', 'ORIG_gmdate');
fb_rename_function('gmdate_ISOLATION_WRAPPER', 'gmdate');

function strftime_ISOLATION_WRAPPER(?string $format, ?int $timestamp = null) {
  $timestamp = ($timestamp === null ? time() : $timestamp);
  return ORIG_strftime($format, $timestamp);
}
fb_rename_function('strftime', 'ORIG_strftime');
fb_rename_function('strftime_ISOLATION_WRAPPER', 'strftime');

function strtotime_ISOLATION_WRAPPER(?string $time, ?int $now = null) {
  $now = ($now === null ? time() : $now);
  return ORIG_strtotime($time, $now);
}
fb_rename_function('strtotime', 'ORIG_strtotime');
fb_rename_function('strtotime_ISOLATION_WRAPPER', 'strtotime');

function idate_ISOLATION_WRAPPER(?string $format, ?int $timestamp = null) {
  $timestamp = ($timestamp === null ? time() : $timestamp);
  return ORIG_idate($format, $timestamp);
}
fb_rename_function('idate', 'ORIG_idate');
fb_rename_function('idate_ISOLATION_WRAPPER', 'idate');

function getdate_ISOLATION_WRAPPER(?int $timestamp = null) {
  $timestamp = ($timestamp === null ? time() : $timestamp);
  return ORIG_getdate($timestamp);
}
fb_rename_function('getdate', 'ORIG_getdate');
fb_rename_function('getdate_ISOLATION_WRAPPER', 'getdate');

function localtime_ISOLATION_WRAPPER(?int $timestamp = null,
                                     bool $is_associative = false) {
  $timestamp = ($timestamp === null ? time() : $timestamp);
  return ORIG_localtime($timestamp, $is_associative);
}
fb_rename_function('localtime', 'ORIG_localtime');
fb_rename_function('localtime_ISOLATION_WRAPPER', 'localtime');


//////////////////////
// Files - Checking //
//////////////////////

fb_intercept(
  'file_get_contents',
  function($name, $obj, $args, $data, &$done) {
    Isolation::CheckReadFile($args[0]);
    $done = false;
  }
);
fb_intercept(
  'fopen',
  function($name, $obj, $args, $data, &$done) {
    $mode = $args[1];
    if (strpbrk($mode, '+waxc')) {
      Isolation::CheckWriteFile($args[0]);
    } else {
      Isolation::CheckReadFile($args[0]);
    }
    $done = false;
  }
);

///////////////////////////
// exec()-like functions //
///////////////////////////

fb_intercept('exec', class_meth('Isolation', 'CheckExec'));
fb_intercept('popen', class_meth('Isolation', 'CheckExec'));
fb_intercept('proc_open', class_meth('Isolation', 'CheckExec'));
fb_intercept('shell_exec', class_meth('Isolation', 'CheckExec'));
fb_intercept('system', class_meth('Isolation', 'CheckExec'));
