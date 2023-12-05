<?hh

namespace {

const int INFO_GENERAL = 1 << 0;
const int INFO_CREDITS = 1 << 1;
const int INFO_CONFIGURATION = 1 << 2;
const int INFO_MODULES = 1 << 3;
const int INFO_ENVIRONMENT = 1 << 4;
const int INFO_VARIABLES = 1 << 5;
const int INFO_LICENSE = 1 << 6;
const int INFO_ALL = -1;

/* Loads the PHP extension given by the parameter library.  Use
 * extension_loaded() to test whether a given extension is already available
 * or not. This works on both built-in extensions and dynamically loaded ones
 * (either through php.ini or dl()). Warning: This function has been removed
 * from some SAPI's in PHP 5.3.
 */
function dl(string $_library): int { return 0; }

/* Finds out whether the extension is loaded.
 */
<<__Native>>
function extension_loaded(string $name)[read_globals]: bool;

/* This function returns the names of all the modules compiled and loaded in
 * the PHP interpreter.
 */
<<__Native>>
function get_loaded_extensions(bool $zend_extensions = false)[read_globals]: varray<string>;

/* This function returns the names of all the functions defined in the module
 * indicated by module_name or false if $module_name is not a valid extension.
 */
<<__Native>>
function get_extension_funcs(string $module_name)[read_globals]: mixed;

/* Gets the value of a PHP configuration option.  This function will not
 * return configuration information set when the PHP was compiled, or read
 * from an Apache configuration file.  To check whether the system is using a
 * configuration file, try retrieving the value of the cfg_file_path
 * configuration setting. If this is available, a configuration file is being
 * used.
 */
function get_cfg_var(string $_option): mixed { return false; }

<<__Native>>
function get_current_user(): string;

/* Returns the names and values of all the constants currently defined. This
 * includes those created by extensions.
 */
<<__Native>>
function get_defined_constants(bool $categorize = false): darray<string, mixed>;

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
function get_included_files(): varray<string>;

function get_required_files(): varray<string> {
  return get_included_files();
}

/* For the duration of the request, keep track of all files that were
 * invovled in autoloading
 */
<<__Native>>
function record_visited_files(): void;

/* Return a list of all files that were involved in autoloading for
 * this request. Recording of visited files starts when the method
 * record_visited_files is called.
 */
<<__Native>>
function get_visited_files(): keyset<string>;

<<__Native>>
function getenv(string $varname)[read_globals]: mixed;

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
function getopt(
  string $options,
  mixed $longopts = null,
): darray<arraykey, mixed>;

/* Similar to getopt but accepts an inout optind which controls the argument
 * at which parsing begins. Sets the optind to the index of the first unparsed
 * option.
 */
<<__Native>>
function getopt_with_optind(string $options,
                            ?vec<string> $longopts,
                            inout int $optind): dict<string, mixed>;

/* This is an interface to getrusage(2). It gets data returned from the system
 * call.
 */
<<__Native>>
function getrusage(int $who = 0): shape(
  'ru_oublock' => int,
  'ru_inblock' => int,
  'ru_msgsnd' => int,
  'ru_msgrcv' => int,
  'ru_maxrss' => int,
  'ru_ixrss' => int,
  'ru_idrss' => int,
  'ru_minflt' => int,
  'ru_majflt' => int,
  'ru_nsignals' => int,
  'ru_nvcsw' => int,
  'ru_nivcsw' => int,
  'ru_nswap' => int,
  'ru_utime_tv_usec' => int,
  'ru_utime_tv_sec' => int,
  'ru_stime_tv_usec' => int,
  'ru_stime_tv_sec' => int,
);

/* Gets resolution of system clock. "man 3 clock_getres" for more details.
 */
<<__Native>>
function clock_getres(int $clk_id,
                      <<__OutOnly('KindOfInt64')>>
                      inout mixed $sec,
                      <<__OutOnly('KindOfInt64')>>
                      inout mixed $nsec): bool;

/* Gets time of a system clock. "man 3 clock_gettime" for more details.
 */
<<__Native>>
function clock_gettime(int $clk_id,
                       <<__OutOnly('KindOfInt64')>>
                       inout mixed $sec,
                       <<__OutOnly('KindOfInt64')>>
                       inout mixed $nsec)[leak_safe]: bool;

/* Same as clock_gettime(), but returns a single integer in nanoseconds.
 * Returns -1 if invalid or non-supported clock is specified.
 */
<<__Native>>
function clock_gettime_ns(int $clk_id)[leak_safe]: int;

/* Gets number of processors.
 */
<<__Native>>
function cpu_get_count()[read_globals]: int;

/* Gets processor model.
 */
<<__Native>>
function cpu_get_model()[read_globals]: string;

/* Returns the value of the configuration option on success.
 */
<<__Native>>
function ini_get(string $varname)[read_globals]: mixed;

/* Gets all configuration options
 */
<<__Native>>
function ini_get_all(
  string $extension = "",
  bool $details = true,
): darray<string, mixed>;

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

/* Sets the value of the given configuration option. The configuration option
 * will keep this new value during the script's execution, and will be
 * restored at the script's ending.
 */
<<__Native>>
function ini_alter(string $varname,
                   mixed $newvalue): mixed;

/* Returns the peak of memory, in bytes, that's been allocated to your PHP
 * script.
 */
<<__Native>>
function memory_get_peak_usage(bool $real_usage = false)[read_globals]: int;

/* Returns the amount of memory, in bytes, that's currently being allocated to
 * your PHP script.
 */
<<__Native>>
function memory_get_usage(bool $real_usage = false)[read_globals]: int;

/* Returns the total memory, in bytes, that your PHP script has allocated.
 */
<<__Native>>
function memory_get_allocation()[read_globals]: int;

/* Returns the request-heap memory currently in use by the script.
 * Does not trigger OOM.
 */
<<__Native>>
function hphp_memory_heap_usage(): int;

/* Returns the current total capacity of the request-heap, including
 * blocks freed by the script but not returned to the process heap,
 * external fragmentation, and heap management overhead.
 * Does not trigger OOM.
 */
<<__Native>>
function hphp_memory_heap_capacity(): int;

/* Returns the peak of memory, in bytes, that's been allocated to your PHP
 * script since calling memory_start_usage_interval.
 */
<<__Native>>
function hphp_memory_get_interval_peak_usage(bool $real_usage = false): int;

/* Starts per-interval usage tracking to allow peak usage to be tracked
 * with more granularity than a per-script basis.
 *
 * Returns whether the state of interval tracking was changed.
 */
<<__Native>>
function hphp_memory_start_interval(): bool;

/* Stops per-interval usage tracking to allow peak usage to be tracked
 * with more granularity than a per-script basis.
 *
 * Returns whether the state of interval tracking was changed.
 */
<<__Native>>
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
function php_sapi_name()[read_globals]: string;

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
  (new \__SystemLib\PhpInfo())->report();
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
 * be protected even if safe_mode_allowed_env_vars is set to allow changing
 * them.
 */
<<__Native>>
function putenv(string $setting): bool;

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

/* Set the number of seconds into the request to invoke a callback.
 * The callback is invoked only once unless another call to this function is
 * made. This can help debugging scripts that take a long time to run.
 */
<<__Native>>
function set_pre_timeout_handler(int $seconds, mixed $callback): void;

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
<<__IsFoldable, __Native>>
function version_compare(string $version1,
                         string $version2,
                         string $sop = "")[]: mixed;

} // root namespace

namespace __SystemLib {

  class PhpInfo {

    private \DOMDocument $xml;
    private \DOMElement $body;

    public function __construct() {
      $this->xml = new \DOMDocument('1.0', 'UTF-8');
      $this->body = $this->element('body');
    }

    private function is_cli(): bool { return \php_sapi_name() == 'cli'; }

    private function appendChildren(
      \DOMElement $el,
      ?varray<mixed> $children,
    ): void {
      if ($children) {
        foreach ($children as $v) {
          if ($v === null) {
          } else if ($v is \DOMElement) {
            $el->appendChild($v);
          } else if (\HH\is_any_array($v)) {
            $this->appendChildren(
              $el,
              HH\FIXME\UNSAFE_CAST<AnyArray<arraykey, mixed>, varray<mixed>>($v),
            );
          } else {
            $el->appendChild(
              HH\FIXME\UNSAFE_CAST<mixed, \DOMNode>(
                $this->xml->createTextNode(
                  HH\FIXME\UNSAFE_CAST<mixed, string>($v),
                )
              )
            );
          }
        }
      }
    }

    private function element(
      string $tag,
      ?darray<string, string> $attr = null,
      mixed... $children
    ): \DOMElement {
      $el = HH\FIXME\UNSAFE_CAST<mixed, \DOMElement>(
        $this->xml->createElement($tag)
      );
      if ($attr) {
        foreach ($attr as $k => $v) {
          $el->setAttribute($k, $v);
        }
      }
      $this->appendChildren($el, $children);
      return $el;
    }

    private function tr(string $l, mixed $d): \DOMElement {
      return
        $this->element(
          'tr', dict[],
          $this->element('td', dict['class' => 'l'], $l),
          $this->element('td', dict['class' => 'r'], $d));
    }

    private function table(
      string $title,
      darray<string, mixed> $data,
    ): ?varray<\DOMElement> {
      if ($this->is_cli()) {
        echo $title . "\n";
        echo "\n";
        foreach ($data as $k => $v) {
          echo $k .
            " => " .
            HH\FIXME\UNSAFE_CAST<mixed, string>(\print_r($v, true)) .
            "\n";
        }
        echo "\n";
        return null;
      } else {
        $children = dict[];
        foreach ($data as $k => $v) {
          \array_push(inout $children, $this->tr($k, \print_r($v, true)));
        }
        return vec[
          $this->element('hr'),
          $this->element('h2', dict[], $title),
          $this->element('table', dict[], $children)
        ];
      }
    }

    private function appendHead(\DOMElement $html): void {
      $style =
        'body { margin: auto; text-align: center; width: 600px; }' .
        'hr { margin-top: 30px; }' .
        'table { border-collapse: collapse; margin: auto; width: 100%; }' .
        'td { border: 1px solid black; padding: 5px; }' .
        '.l { background-color: #CCF; }' .
        '.r { background-color: #CCC; word-break: break-all; }';

      $html->appendChild(
        $this->element(
          'head', dict[],
          $this->element('title', dict[], "HHVM phpinfo"),
          $this->element('style', dict['type' => 'text/css'], $style)));
    }

    private function reportVersionTitle(): void {
      if ($this->is_cli()) {
        echo 'HHVM Version => ' . \HHVM_VERSION . "\n";
      } else {
        $this->body->appendChild(
          $this->element('h1', dict[], 'HHVM Version ' . \HHVM_VERSION));
      }
    }

    private function reportVersions(): void {
      if (!$this->is_cli()) {
        $this->body->appendChild($this->element('h2', dict[], 'Version'));
      }

      $data = dict[
        'Version' => \HHVM_VERSION,
        'Version ID' => \HHVM_VERSION_ID,
        'Debug' => \HHVM_DEBUG,
        'Compiler ID' => \HHVM_COMPILER_ID,
        'Repo Schema' => \HHVM_REPO_SCHEMA,
        'PHP Version' => \phpversion(),
        'uname' => \php_uname()];

      $this->appendChildren($this->body, $this->table('Version', $data));
    }

    private function reportIni(): void {
      $this->appendChildren($this->body,
                            $this->table('INI', \ini_get_all('', false)));
    }

    private function reportHeaders(): void {
      if (!\function_exists('HH\\get_headers_secure')) return;
      $headers = dict[];
      foreach (\HH\get_headers_secure() as $k => $vs) {
        // preserve "last seen header" behavior of `getallheaders`
        $headers[$k] = $vs[\count($vs) - 1];
      }
      $this->appendChildren($this->body,
                            $this->table('Headers', $headers));
    }

    private function reportMap(string $name, darray<string, mixed> $map): void {
      $data = dict[];
      foreach ($map as $k => $v) {
        $data[
          HH\FIXME\UNSAFE_CAST<mixed, string>(\sprintf("%s['%s']", $name, $k))
        ] = $v;
      }
      $this->appendChildren($this->body, $this->table($name, $data));
    }

    public function report(): void {

      $html = $this->element('html');

      if (!$this->is_cli()) {
        $this->appendHead($html);
        $html->appendChild($this->body);
      }

      $this->reportVersionTitle();

      if (!$this->is_cli()) {
        $this->body->appendChild($this->element('hr'));
      }

      $this->reportVersions();
      $this->reportIni();
      $this->reportHeaders();

      $this->reportMap(
        '$_SERVER',
        HH\FIXME\UNSAFE_CAST<mixed, darray<string, mixed>>(
          \HH\global_get('_SERVER')
        ),
      );
      $this->reportMap(
        '$_ENV',
        HH\FIXME\UNSAFE_CAST<mixed, darray<string, mixed>>(
          \HH\global_get('_ENV')
        ),
      );

      if (!$this->is_cli()) {
        $this->body->appendChild($this->element('br'));
        $this->xml->appendChild($html);
        \header('content-type: text/html; charset=UTF-8');
        echo HH\FIXME\UNSAFE_CAST<mixed, arraykey>($this->xml->saveHTML());
      }
    }
  }
}
