<?hh

class finfo {

  private $resource;

  private $options;
  private $magic_file;

  /**
   * Create a new fileinfo resource
   *
   * @param int $options - One or disjunction of more Fileinfo constants.
   * @param string $magic_file - Name of a magic database file, usually
   *   something like /path/to/magic.mime. If not specified, the MAGIC
   *   environment variable is used. If the environment variable isn't set,
   *   then PHP's bundled magic database will be used.   Passing NULL or an
   *   empty string will be equivalent to the default value.
   */
  public function __construct(int $options = FILEINFO_NONE,
                              ?string $magic_file = NULL) {
    $this->resource = finfo_open($options, $magic_file);

    $this->options = $options;
    $this->magic_file = $magic_file;
  }

  public function __sleep() {
    return vec['options', 'magic_file'];
  }

  public function __wakeup() {
    $this->resource = finfo_open($this->options, $this->magic_file);
  }

  /**
   * Return information about a string buffer
   *
   * @param string $string - Content of a file to be checked.
   * @param int $options - One or disjunction of more Fileinfo constants.
   * @param resource $context -
   *
   * @return string - Returns a textual description of the string
   *   argument, or FALSE if an error occurred.
   */
  public function buffer(?string $string = NULL,
                         int $options = FILEINFO_NONE,
                         ?resource $context = NULL): string {
    return finfo_buffer($this->resource, $string, $options, $context);
  }

  /**
   * Return information about a file
   *
   * @param string $file_name - Name of a file to be checked.
   * @param int $options - One or disjunction of more Fileinfo constants.
   * @param resource $context - For a description of contexts, refer to .
   *
   * @return string - Returns a textual description of the contents of
   *   the filename argument, or FALSE if an error occurred.
   */
  public function file(?string $file_name = NULL,
                       int $options = FILEINFO_NONE,
                       ?resource $context = NULL): string {
    return finfo_file($this->resource, $file_name, $options, $context);
  }

  /**
   * Set libmagic configuration options
   *
   * @param int $options - One or disjunction of more Fileinfo constants.
   *
   * @return bool -
   */
  public function set_flags(int $options): bool {
    $ret = finfo_set_flags($this->resource, $options);
    if ($ret) {
      $this->options = $options;
    }
    return $ret;
  }

}

/**
 * Return information about a string buffer
 *
 * @param resource $finfo - Fileinfo resource returned by finfo_open().
 * @param string $string - Content of a file to be checked.
 * @param int $options - One or disjunction of more Fileinfo constants.
 * @param resource $context -
 *
 * @return string - Returns a textual description of the string argument,
 *   or FALSE if an error occurred.
 */
<<__Native>>
function finfo_buffer(resource $finfo,
                      ?string $string = NULL,
                      int $options = FILEINFO_NONE,
                      ?resource $context = NULL): string;

/**
 * Close fileinfo resource
 *
 * @param resource $finfo - Fileinfo resource returned by finfo_open().
 *
 * @return bool -
 */
<<__Native>>
function finfo_close(resource $finfo): bool;

/**
 * Return information about a file
 *
 * @param resource $finfo - Fileinfo resource returned by finfo_open().
 * @param string $file_name - Name of a file to be checked.
 * @param int $options - One or disjunction of more Fileinfo constants.
 * @param resource $context - For a description of contexts, refer to .
 *
 * @return string - Returns a textual description of the contents of the
 *   filename argument, or FALSE if an error occurred.
 */
<<__Native>>
function finfo_file(resource $finfo,
                    ?string $file_name = NULL,
                    int $options = FILEINFO_NONE,
                    ?resource $context = NULL): string;

/**
 * Create a new fileinfo resource
 *
 * @param int $options - One or disjunction of more Fileinfo constants.
 * @param string $magic_file - Name of a magic database file, usually
 *   something like /path/to/magic.mime. If not specified, the MAGIC
 *   environment variable is used. If the environment variable isn't set,
 *   then PHP's bundled magic database will be used.   Passing NULL or an
 *   empty string will be equivalent to the default value.
 *
 * @return resource - (Procedural style only) Returns a magic database
 *   resource on success.
 */
<<__Native>>
function finfo_open(int $options = FILEINFO_NONE,
                    ?string $magic_file = NULL): mixed;

/**
 * Set libmagic configuration options
 *
 * @param resource $finfo - Fileinfo resource returned by finfo_open().
 * @param int $options - One or disjunction of more Fileinfo constants.
 *
 * @return bool -
 */
<<__Native>>
function finfo_set_flags(resource $finfo,
                         int $options): bool;

/**
 * Detect MIME Content-type for a file (deprecated)
 *
 * @param string $filename - Path to the tested file.
 *
 * @return string - Returns the content type in MIME format, like
 *   text/plain or application/octet-stream.
 */
<<__Native>>
function mime_content_type(mixed $filename): string;
