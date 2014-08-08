<?hh

namespace {
  /**
   * SessionHandler a special class that can be used to expose the current
   * internal PHP session save handler by inheritance. There are six methods
   * which wrap the six internal session save handler callbacks (open, close,
   * read, write, destroy and gc). By default, this class will wrap whatever
   * internal save handler is set as as defined by the session.save_handler
   * configuration directive which is usually files by default. Other internal
   * session save handlers are provided by PHP extensions such as SQLite (as
   * sqlite), Memcache (as memcache), and Memcached (as memcached).   When a
   * plain instance of SessionHandler is set as the save handler using
   * session_set_save_handler() it will wrap the current save handlers. A class
   * extending from SessionHandler allows you to override the methods or
   * intercept or filter them by calls the parent class methods which ultimately
   * wrap the interal PHP session handlers.   This allows you, for example, to
   * intercept the read and write methods to encrypt/decrypt the session data
   * and then pass the result to and from the parent class. Alternatively one
   * might chose to entirely override a method like the garbage collection
   * callback gc.   Because the SessionHandler wraps the current internal save
   * handler methods, the above example of encryption can be applied to any
   * internal save handler without having to know the internals of the handlers.
   *   To use this class, first set the save handler you wish to expose using
   * session.save_handler and then pass an instance of SessionHandler or one
   * extending it to session_set_save_handler().   Please note the callback
   * methods of this class are designed to be called internally by PHP and are
   * not meant to be called from user-space code. The return values are equally
   * processed internally by PHP. For more information on the session workflow,
   * please refer session_set_save_handler().
   */
  class SessionHandler implements SessionHandlerInterface {
    <<__Native, __HipHopSpecific>>
    private function hhclose(): bool;

    /**
     * Close the session
     *
     * @return bool -
     */
    public function close() {
      return $this->hhclose();
    }

    <<__Native, __HipHopSpecific>>
    private function hhdestroy(string $session_id): bool;

    /**
     * Destroy a session
     *
     * @param string $session_id - The session ID being destroyed.
     *
     * @return bool -
     */
    public function destroy($session_id) {
      return $this->hhdestroy($session_id);
    }

   <<__Native, __HipHopSpecific>>
    private function hhgc(int $maxlifetime): bool;

    /**
     * Cleanup old sessions
     *
     * @param int $maxlifetime -
     *
     * @return bool -
     */
    public function gc($maxlifetime) {
      return $this->hhgc($maxlifetime);
    }

    <<__Native, __HipHopSpecific>>
    private function hhopen(string $save_path, string $session_id): bool;

    /**
     * Initialize session
     *
     * @param string $save_path -
     * @param string $session_id -
     *
     * @return bool -
     */
    public function open($save_path, $session_id) {
      return $this->hhopen($save_path, $session_id);
    }


    <<__Native, __HipHopSpecific>>
    private function hhread(string $session_id): ?string;

    /**
     * Read session data
     *
     * @param string $session_id -
     *
     * @return string - Returns an encoded string of the read data. If
     *   nothing was read, it must return an empty string. Note this value is
     *   returned internally to PHP for processing.
     */
    public function read($session_id) {
      return $this->hhread($session_id);
    }


    <<__Native, __HipHopSpecific>>
    private function hhwrite(string $session_id, string $data): bool;

    /**
     * Write session data
     *
     * @param string $session_id -
     * @param string $session_data -
     *
     * @return bool -
     */
    public function write($session_id, $session_data) {
      return $this->hhwrite($session_id, $session_data);
    }

  } // Class

  /**
   * Return current cache expire
   *
   * @param string $new_cache_expire - If new_cache_expire is given, the
   *   current cache expire is replaced with new_cache_expire.     Setting
   *   new_cache_expire is of value only, if session.cache_limiter is set to
   *   a value different from nocache.
   *
   * @return int - Returns the current setting of session.cache_expire. The
   *   value returned should be read in minutes, defaults to 180.
   */
  function session_cache_expire(mixed $new_cache_expire = null): int {
    $ret = (int)ini_get('session.cache_expire');
    if ($new_cache_expire !== null) {
      $val = (string)$new_cache_expire;
      ini_set('session.cache_expire', (int)$val);
    }
    return $ret;
  }

  /**
   * Get and/or set the current cache limiter
   *
   * @param string $cache_limiter - If cache_limiter is specified, the name
   *   of the current cache limiter is changed to the new value.   Possible
   *   values    Value Headers sent     public        private_no_expire
   *    private        nocache
   *
   * @return string - Returns the name of the current cache limiter.
   */
  function session_cache_limiter(mixed $cache_limiter = null): string {
    $ret = (string)ini_get('session.cache_limiter');
    if ($cache_limiter !== null) {
      ini_set('session.cache_limiter', (string)$cache_limiter);
    }
    return $ret;
  }

  /**
   * Alias of session_write_close()
   */
  function session_commit(): void {
    session_write_close();
  }

  /**
   * Decodes session data from a session encoded string
   *
   * @param string $data - The encoded data to be stored.
   *
   * @return bool -
   */
  <<__Native>>
  function session_decode(string $data): bool;

  /**
   * Destroys all data registered to a session
   *
   * @return bool -
   */
  <<__Native>>
  function session_destroy(): bool;

  /**
   * Encodes the current session data as a session encoded string
   *
   * @return string - Returns the contents of the current session encoded.
   */
  <<__Native>>
  function session_encode(): mixed;

  /**
   * Get the session cookie parameters
   *
   * @return array - Returns an array with the current session cookie
   *   information, the array contains the following items:    "lifetime" -
   *   The lifetime of the cookie in seconds.     "path" - The path where
   *   information is stored.     "domain" - The domain of the cookie.
   *   "secure" - The cookie should only be sent over secure connections.
   *   "httponly" - The cookie can only be accessed through the HTTP
   *   protocol.
   */
  function session_get_cookie_params(): array<string, mixed> {
    return array(
      'lifetime' => (int)ini_get('session.cookie_lifetime'),
      'path'     => (string)ini_get('session.cookie_path'),
      'domain'   => (string)ini_get('session.cookie_domain'),
      'secure'   => (bool)ini_get('session.cookie_secure'),
      'httponly' => (bool)ini_get('session.cookie_httponly')
    );
  }

  /**
   * Get and/or set the current session id
   *
   * @param string $id - If id is specified, it will replace the current
   *   session id. session_id() needs to be called before session_start() for
   *   that purpose. Depending on the session handler, not all characters are
   *   allowed within the session id. For example, the file session handler
   *   only allows characters in the range a-z A-Z 0-9 , (comma) and -
   *   (minus)!    When using session cookies, specifying an id for
   *   session_id() will always send a new cookie when session_start() is
   *   called, regardless if the current session id is identical to the one
   *   being set.
   *
   * @return string - session_id() returns the session id for the current
   *   session or the empty string ("") if there is no current session (no
   *   current session id exists).
   */
  <<__Native>>
  function session_id(?string $id = null): string;

  /**
   * Get and/or set the current session module
   *
   * @param string $module - If module is specified, that module will be
   *   used instead.
   *
   * @return string - Returns the name of the current session module.
   */
  <<__Native>>
  function session_module_name(?string $module = null): mixed;

  /**
   * Get and/or set the current session name
   *
   * @param string $name - The session name references the name of the
   *   session, which is used in cookies and URLs (e.g. PHPSESSID). It should
   *   contain only alphanumeric characters; it should be short and
   *   descriptive (i.e. for users with enabled cookie warnings). If name is
   *   specified, the name of the current session is changed to its value.
   *    The session name can't consist of digits only, at least one letter
   *   must be present. Otherwise a new session id is generated every time.
   *
   * @return string - Returns the name of the current session.
   */
  function session_name(mixed $name = null): string {
    $oldname = (string)ini_get('session.name');
    if ($name !== null) {
      ini_set('session.name', (string)$name);
    }
    return $oldname;
  }

  /**
   * Update the current session id with a newly generated one
   *
   *
   * @param bool $delete_old_session - Whether to delete the old associated
   *   session file or not.
   *
   * @return bool -
   */
  <<__Native>>
  function session_regenerate_id(bool $delete_old_session = false): bool;

  /**
   * Session shutdown function
   *
   * @return void -
   */
  function session_register_shutdown(): void {
    register_shutdown_function('session_write_close');
  }

  /**
   * Get and/or set the current session save path
   *
   * @param string $path - Session data path. If specified, the path to
   *   which data is saved will be changed. session_save_path() needs to be
   *   called before session_start() for that purpose.     On some operating
   *   systems, you may want to specify a path on a filesystem that handles
   *   lots of small files efficiently. For example, on Linux, reiserfs may
   *   provide better performance than ext2fs.
   *
   * @return string - Returns the path of the current directory used for
   *   data storage.
   */
  function session_save_path(mixed $path = null): mixed {
    if ($path !== null) {
      $path = (string)$path;
      if (strpos($path, "\0") !== false) {
        trigger_error('The save_path cannot contain NULL characters',
                      E_WARNING);
        return false;
      }
      ini_set('session.save_path', $path);
    }
    return ini_get('session.save_path');
  }

  /**
   * Set the session cookie parameters
   *
   * @param int $lifetime - Lifetime of the session cookie, defined in
   *   seconds.
   * @param string $path - Path on the domain where the cookie will work.
   *   Use a single slash ('/') for all paths on the domain.
   * @param string $domain - Cookie domain, for example 'www.php.net'. To
   *   make cookies visible on all subdomains then the domain must be
   *   prefixed with a dot like '.php.net'.
   * @param bool $secure - If TRUE cookie will only be sent over secure
   *   connections.
   * @param bool $httponly - If set to TRUE then PHP will attempt to send
   *   the httponly flag when setting the session cookie.
   *
   * @return void -
   */
  function session_set_cookie_params(mixed $lifetime,
                                     mixed $path = null,
                                     mixed $domain = null,
                                     mixed $secure = null,
                                     mixed $httponly = null): void {
    if (is_object($lifetime)) {
      trigger_error(sprintf(
        'Notice: Object of class %s could not be converted to int',
        get_class($lifetime)),
      E_NOTICE);
      return;
    }
    if (ini_get('session.use_cookies')) {
      ini_set('session.cookie_lifetime', (int)$lifetime);
      if ($path !== null) {
        ini_set('session.cookie_path', (string)$path);
      }
      if ($domain !== null) {
        ini_set('session.cookie_domain', (string)$domain);
      }
      if ($secure !== null) {
        ini_set('session.cookie_secure', (bool)$secure);
      }
      if ($httponly !== null) {
        ini_set('session.cookie_httponly', (bool)$httponly);
      }
    }
  }

  /**
   * Sets user-level session storage functions
   *
   * @param sessionhandlerinterface $sessionhandler - An instance of a
   *   class implementing SessionHandlerInterface, such as SessionHandler, to
   *   register as the session handler. Since PHP 5.4 only.
   * @param bool $register_shutdown - Register session_write_close() as a
   *   register_shutdown_function() function.
   *
   * @return bool -
   */
  function session_set_save_handler(mixed $open, mixed $close = null,
                                    mixed $read = null, mixed $write = null,
                                    mixed $destroy = null,
                                    mixed $gc = null): bool {
    if ($open instanceof SessionHandlerInterface) {
      return \__SystemLib\session_set_save_handler($open, $close);
    }
    return \__SystemLib\session_set_save_handler(
      new \__SystemLib\SessionForwardingHandler($open, $close, $read, $write,
                                                $destroy, $gc),
      false
    );
  }

  /**
   * Start new or resume existing session
   *
   * @return bool - This function returns TRUE if a session was
   *   successfully started, otherwise FALSE.
   */
  <<__Native>>
  function session_start(): bool;

  /**
   * Returns the current session status
   *
   * @return int - PHP_SESSION_DISABLED if sessions are disabled.
   *   PHP_SESSION_NONE if sessions are enabled, but none exists.
   *   PHP_SESSION_ACTIVE if sessions are enabled, and one exists.
   */
  <<__Native>>
  function session_status(): int;

  /**
   * Free all session variables
   *
   * @return void -
   */
  <<__Native>>
  function session_unset(): void;

  /**
   * Write session data and end session
   *
   * @return void -
   */
  <<__Native>>
  function session_write_close(): void;
} // Namespace (global)


namespace __SystemLib {
  <<__Native("NoInjection"), __HipHopSpecific>>
  function session_set_save_handler(
    \SessionHandlerInterface $sessionhandler,
    bool $register_shutdown = true) : bool;

  class SessionForwardingHandler implements \SessionHandlerInterface {
    private $open;
    private $close;
    private $read;
    private $write;
    private $destory;
    private $gc;

    public function __construct($open, $close, $read, $write, $destroy, $gc)  {
      try {
        $this->open = $this->validate($open, 1);
        $this->close = $this->validate($close, 2);
        $this->read = $this->validate($read, 3);
        $this->write = $this->validate($write, 4);
        $this->destroy = $this->validate($destroy, 5);
        $this->gc = $this->validate($gc, 6);
      } catch (\Exception $e) {
        trigger_error($e->getMessage(), E_WARNING);
        return false;
      }
    }

    public function open($save_path, $session_id) {
      if ($this->open) {
        return call_user_func($this->open, $save_path, $session_id);
      }
    }
    public function close() {
      if ($this->close) {
        return call_user_func($this->close);
      }
    }
    public function read($session_id) {
      if ($this->read) {
        return call_user_func($this->read, $session_id);
      }
    }
    public function write($session_id, $session_data) {
      if ($this->write) {
        return call_user_func($this->write, $session_id, $session_data);
      }
    }
    public function destroy($session_id) {
      if ($this->destroy) {
        return call_user_func($this->destroy, $session_id);
      }
    }
    public function gc($maxlifetime) {
      if ($this->gc) {
        return call_user_func($this->gc, $maxlifetime);
      }
    }
    private function validate($func, $num) {
      if (!is_callable($func)) {
        throw new \Exception("Argument $num is not a valid callback");
      }
      return $func;
    }
  } // Class
} // Namespace (__SystemLib)
