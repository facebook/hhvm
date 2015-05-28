<?hh

/* Set the various assert() control options or just query their current
 * settings.
 */
<<__Native>>
function assert_options(int $what,
                        mixed $value = null): mixed;

/* assert() will check the given assertion and take appropriate action if its
 * result is FALSE.  If the assertion is given as a string it will be
 * evaluated as PHP code by assert(). The advantages of a string assertion are
 * less overhead when assertion checking is off and messages containing the
 * assertion expression when an assertion fails. This means that if you pass a
 * boolean condition as assertion this condition will not show up as parameter
 * to the assertion function which you may have defined with the
 * assert_options() function, the condition is converted to a string before
 * calling that handler function, and the boolean FALSE is converted as the
 * empty string.  Assertions should be used as a debugging feature only. You
 * may use them for sanity-checks that test for conditions that should always
 * be TRUE and that indicate some programming errors if not or to check for
 * the presence of certain features like extension functions or certain system
 * limits and features.  Assertions should not be used for normal runtime
 * operations like input parameter checks. As a rule of thumb your code should
 * always be able to work correctly if assertion checking is not activated.
 * The behavior of assert() may be configured by assert_options() or by
 * .ini-settings described in that functions manual page.  The
 * assert_options() function and/or ASSERT_CALLBACK configuration directive
 * allow a callback function to be set to handle failed assertions.  assert()
 * callbacks are particularly useful for building automated test suites
 * because they allow you to easily capture the code passed to the assertion,
 * along with information on where the assertion was made. While this
 * information can be captured via other methods, using assertions makes it
 * much faster and easier!  The callback function should accept three
 * arguments. The first argument will contain the file the assertion failed
 * in. The second argument will contain the line the assertion failed on and
 * the third argument will contain the expression that failed (if any -
 * literal values such as 1 or "two" will not be passed via this argument)
 */
<<__Native>>
function assert(mixed $assertion, ?string $message = null): mixed;

/* Loads the PHP extension given by the parameter library.  Use
 * extension_loaded() to test whether a given extension is already available
 * or not. This works on both built-in extensions and dynamically loaded ones
 * (either through php.ini or dl()). Warning: This function has been removed
 * from some SAPI's in PHP 5.3.
 */
<<__Native>>
function dl(string $library): int;

/* Finds out whether the extension is loaded.
 */
<<__Native>>
function extension_loaded(string $name): bool;

/* This function returns the names of all the modules compiled and loaded in
 * the PHP interpreter.
 */
<<__Native>>
function get_loaded_extensions(bool $zend_extensions = false): array;

/* This function returns the names of all the functions defined in the module
 * indicated by module_name.
 */
<<__Native>>
function get_extension_funcs(string $module_name): array;

/* Gets the value of a PHP configuration option.  This function will not
 * return configuration information set when the PHP was compiled, or read
 * from an Apache configuration file.  To check whether the system is using a
 * configuration file, try retrieving the value of the cfg_file_path
 * configuration setting. If this is available, a configuration file is being
 * used.
 */
<<__Native>>
function get_cfg_var(string $option): mixed;

<<__Native>>
function get_current_user(): string;

/* Returns the names and values of all the constants currently defined. This
 * includes those created by extensions as well as those created with the
 * define() function.
 */
<<__Native>>
function get_defined_constants(bool $categorize = false): array;

<<__Native>>
function get_include_path(): string;

<<__Native>>
function restore_include_path(): void;

/* Sets the include_path configuration option for the duration of the script.
 */
<<__Native>>
function set_include_path(mixed $new_include_path): string;

/* Gets the names of all files that have been included using include(),
 * include_once(), require() or require_once().
 */
<<__Native>>
function get_included_files(): array;

function get_required_files(): array {
  return get_included_files();
}

/* Returns the current configuration setting of magic_quotes_gpc  Keep in mind
 * that attempting to set magic_quotes_gpc at runtime will not work.  For more
 * information about magic_quotes, see this security section.
 */
function get_magic_quotes_gpc(): ?bool {
  if (($argc = func_num_args()) != 0) {
    trigger_error(__FUNCTION__ . "() expects exactly 0 parameters," .
                  " $argc given", E_USER_WARNING);
    return null;
  }
  return false;
}

function get_magic_quotes_runtime(): ?bool {
  if (($argc = func_num_args()) != 0) {
    trigger_error(__FUNCTION__ . "() expects exactly 0 parameters," .
                  " $argc given", E_USER_WARNING);
    return null;
  }
  return false;
}

<<__Native>>
function getenv(string $varname): mixed;

/* Gets the time of the last modification of the current page.  If you're
 * interested in getting the last modification time of a different file,
 * consider using filemtime().
 */
<<__Native>>
function getlastmod(): mixed;

<<__Native>>
function getmygid(): mixed;

/* Gets the inode of the current script.
 */
<<__Native>>
function getmyinode(): mixed;

/* Gets the current PHP process ID.
 */
<<__Native>>
function getmypid(): mixed;

<<__Native>>
function getmyuid(): mixed;

/* Parses options passed to the script. The options parameter may contain the
 * following elements: Individual characters (do not accept values) Characters
 * followed by a colon (parameter requires value) Characters followed by two
 * colons (optional value) Option values are the first argument after the
 * string. It does not matter if a value has leading white space or not.
 * Optional values do not accept " " (space) as a separator.  The format for
 * the options and longopts is almost the same, the only difference is that
 * longopts takes an array of options (where each element is the option) where
 * as options takes a string (where each character is the option).
 */
<<__Native>>
function getopt(string $options,
                mixed $longopts = null): array;

/* This is an interface to getrusage(2). It gets data returned from the system
 * call.
 */
<<__Native>>
function getrusage(int $who = 0): array;

/* Gets resolution of system clock. "man 3 clock_getres" for more details.
 */
<<__Native>>
function clock_getres(int $clk_id,
                      mixed &$sec,
                      mixed &$nsec): bool;

/* Gets time of a system clock. "man 3 clock_gettime" for more details.
 */
<<__Native>>
function clock_gettime(int $clk_id,
                       mixed &$sec,
                       mixed &$nsec): bool;

/* Gets number of processors.
 */
<<__Native>>
function cpu_get_count(): int;

/* Gets processor model.
 */
<<__Native>>
function cpu_get_model(): string;

/* Returns the value of the configuration option on success.
 */
<<__Native>>
function ini_get(string $varname): mixed;

/* Gets all configuration options
 */
<<__Native>>
function ini_get_all(string $extension = "",
                     bool $details = true): array;

/* Restores a given configuration option to its original value.
 */
<<__Native>>
function ini_restore(string $varname): void;

/* Sets the value of the given configuration option. The configuration option
 * will keep this new value during the script's execution, and will be
 * restored at the script's ending.
 */
<<__Native>>
function ini_set(string $varname,
                 mixed $newvalue): mixed;

/* Returns the peak of memory, in bytes, that's been allocated to your PHP
 * script.
 */
<<__Native>>
function memory_get_peak_usage(bool $real_usage = false): int;

/* Returns the amount of memory, in bytes, that's currently being allocated to
 * your PHP script.
 */
<<__Native>>
function memory_get_usage(bool $real_usage = false): int;

/* Returns the total memory, in bytes, that your PHP script has allocated.
 */
<<__Native, __HipHopSpecific>>
function memory_get_allocation(): int;

/* Returns the peak of memory, in bytes, that's been allocated to your PHP
 * script since calling memory_start_usage_interval.
 */
<<__Native, __HipHopSpecific>>
function hphp_memory_get_interval_peak_usage(bool $real_usage = false): int;

/* Starts per-interval usage tracking to allow peak usage to be tracked
 * with more granularity than a per-script basis.
 *
 * Returns whether the state of interval tracking was changed.
 */
<<__Native, __HipHopSpecific>>
function hphp_memory_start_interval(): bool;

/* Stops per-interval usage tracking to allow peak usage to be tracked
 * with more granularity than a per-script basis.
 *
 * Returns whether the state of interval tracking was changed.
 */
<<__Native, __HipHopSpecific>>
function hphp_memory_stop_interval(): bool;

/* Retrieve a path to the loaded php.ini file.
 */
function php_ini_loaded_file(): mixed {
  return false;
}

/* Retrieve a comma-separated list of paths to any additionally loaded ini
 * files after php.ini.
 */
function php_ini_scanned_files(): mixed {
  return false;
}

<<__Native>>
function php_sapi_name(): string;

/* php_uname() returns a description of the operating system PHP is running
 * on. This is the same string you see at the very top of the phpinfo()
 * output. For the name of just the operating system, consider using the
 * PHP_OS constant, but keep in mind this constant will contain the operating
 * system PHP was built on.  On some older UNIX platforms, it may not be able
 * to determine the current OS information in which case it will revert to
 * displaying the OS PHP was built on. This will only happen if your uname()
 * library call either doesn't exist or doesn't work.
 */
<<__Native>>
function php_uname(string $mode = ""): mixed;

/* Outputs a large amount of information about the current state of PHP. This
 * includes information about PHP compilation options and extensions, the PHP
 * version, server information and environment (if compiled as a module), the
 * PHP environment, OS version information, paths, master and local values of
 * configuration options, HTTP headers, and the PHP License.  Because every
 * system is setup differently, phpinfo() is commonly used to check
 * configuration settings and for available predefined variables on a given
 * system.  phpinfo() is also a valuable debugging tool as it contains all
 * EGPCS (Environment, GET, POST, Cookie, Server) data.
 */
function phpinfo(int $what = 0): bool {
  echo '<html>';

  echo '<head>';
  echo '<title>HHVM phpinfo</title>';
  echo '<style type="text/css">';
  echo 'body { margin: auto; text-align: center; width: 600px; }';
  echo 'hr { margin-top: 30px; }';
  echo 'table { border-collapse: collapse; margin: auto; width: 100%; }';
  echo 'td { border: 1px solid black; padding: 5px; }';
  echo '.l { background-color: #CCF; }';
  echo '.r { background-color: #CCC; word-break: break-all; }';
  echo '</style>';
  echo '</head>';

  echo '<body>';

  echo '<h1>HHVM Version '.HHVM_VERSION.'</h1>';
  echo '<hr>';

  echo '<h2>Version</h2>';
  echo '<table>';
  \__SystemLib\phpinfo_tr('Version', HHVM_VERSION);
  \__SystemLib\phpinfo_tr('Version ID', HHVM_VERSION_ID);
  \__SystemLib\phpinfo_tr('Debug', HHVM_DEBUG);
  \__SystemLib\phpinfo_tr('Compiler ID', HHVM_COMPILER_ID);
  \__SystemLib\phpinfo_tr('Repo Schema', HHVM_REPO_SCHEMA);
  \__SystemLib\phpinfo_tr('PHP Version', phpversion());
  \__SystemLib\phpinfo_tr('Zend Version', zend_version());
  \__SystemLib\phpinfo_tr('uname', php_uname());
  echo '</table>';

  \__SystemLib\phpinfo_table('INI', ini_get_all(null, false));

  if (function_exists('getallheaders')) {
    \__SystemLib\phpinfo_table('Headers', getallheaders());
  }

  \__SystemLib\phpinfo_table('$_SERVER', $_SERVER);
  \__SystemLib\phpinfo_table('$_ENV', $_ENV);

  echo '</body>';
  echo '</html>';
  return true;
}

/* Returns a string containing the version of the currently running PHP parser
 * or extension.
 */
<<__Native>>
function phpversion(string $extension = ""): mixed;

/* Adds setting to the server environment. The environment variable will only
 * exist for the duration of the current request. At the end of the request
 * the environment is restored to its original state.  Setting certain
 * environment variables may be a potential security breach. The
 * safe_mode_allowed_env_vars directive contains a comma-delimited list of
 * prefixes. In Safe Mode, the user may only alter environment variables whose
 * names begin with the prefixes supplied by this directive. By default, users
 * will only be able to set environment variables that begin with PHP_ (e.g.
 * PHP_FOO=BAR). Note: if this directive is empty, PHP will let the user
 * modify ANY environment variable!  The safe_mode_protected_env_vars
 * directive contains a comma-delimited list of environment variables, that
 * the end user won't be able to change using putenv(). These variables will
 * be protected even if safe_mode_allowed_env_vars is set to allow to change
 * them.
 */
<<__Native>>
function putenv(string $setting): bool;

/* Set the current active configuration setting of magic_quotes_runtime.
 * Warning: This function has been DEPRECATED as of PHP 5.3.0. Relying on this
 * feature is highly discouraged.
 */
function set_magic_quotes_runtime(mixed $new_setting): bool {
  trigger_error("Function set_magic_quotes_runtime() is deprecated",
                E_USER_DEPRECATED);

  if ($new_setting) {
    trigger_error(__FUNCTION__ . "() is not supported anymore", E_USER_ERROR);
  }

  return false;
}

/*
 * Alias of set_magic_quotes_runtime()
 */
function magic_quotes_runtime(mixed $new_setting): bool {
  return set_magic_quotes_runtime($new_setting);
}

/* Set the number of seconds a script is allowed to run. If this is reached,
 * the script returns a fatal error. The default limit is 30 seconds or, if it
 * exists, the max_execution_time value defined in the php.ini.  When called,
 * set_time_limit() restarts the timeout counter from zero. In other words, if
 * the timeout is the default 30 seconds, and 25 seconds into script execution
 * a call such as set_time_limit(20) is made, the script will run for a total
 * of 45 seconds before timing out.
 */
<<__Native>>
function set_time_limit(int $seconds): void;

/* Returns the path of the directory PHP stores temporary files in by default.
 */
<<__Native>>
function sys_get_temp_dir(): string;

/* version_compare() compares two "PHP-standardized" version number strings.
 * This is useful if you would like to write programs working only on some
 * versions of PHP.  The function first replaces _, - and + with a dot . in
 * the version strings and also inserts dots . before and after any non number
 * so that for example '4.3.2RC1' becomes '4.3.2.RC.1'. Then it splits the
 * results like if you were using explode('.', $ver). Then it compares the
 * parts starting from left to right. If a part contains special version
 * strings these are handled in the following order: any string not found in
 * this list < dev < alpha = a < beta = b < RC = rc < # < pl = p. This way not
 * only versions with different levels like '4.1' and '4.1.2' can be compared
 * but also any PHP specific version containing development state.
 */
<<__Native>>
function version_compare(string $version1,
                         string $version2,
                         string $sop = ""): mixed;

/* Returns a string containing the version of the currently running Zend
 * Engine.
 */
<<__Native>>
function zend_version(): string;

namespace __SystemLib {
  <<__Native>>
  function assert(mixed $assertion, ?string $message = null): ?bool;

  function phpinfo_tr($l, $d) {
    echo "<tr><td class=\"l\">$l</td><td class=\"r\">$d</td></tr>";
  }

  function phpinfo_table($l, $a) {
    echo '<hr>';
    echo "<h2>$l</h2>";
    echo '<table>';
    foreach ($a as $k => $v) {
      phpinfo_tr($k, print_r($v, true));
    }
    echo '</table>';
  }
}
