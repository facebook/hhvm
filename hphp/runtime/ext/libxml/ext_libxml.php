<?hh

class LibXMLError {
  public $level;
  public $code;
  public $column;
  public $message;
  public $file;
  public $line;
}

/* Retrieve array of errors.
 *
 * Returns an array with LibXMLError objects if there are any errors in the
 * buffer, or an empty array otherwise.
 */
<<__Native>>
function libxml_get_errors(): varray;

/* Returns a LibXMLError object if there is any error in the buffer, FALSE
 * otherwise.
 */
<<__Native>>
function libxml_get_last_error(): mixed;

/* Clears the libxml error buffer. */
<<__Native>>
function libxml_clear_errors(): void;

/* Allows you to disable standard libxml errors and enable user error
 * handling.
 */
<<__Native>>
function libxml_use_internal_errors(bool $use_errors = false): bool;

/* Suppress recording of internal error recording to save memory.
 * Has no effect unless libxml_use_internal_errors() is enabled.
 * Reset when libxml_use_internal_errors() is called.
 */
<<__Native>>
function libxml_suppress_errors(bool $suppress_errors): void;

/* Disable/enable the ability to load external entities. */
<<__Native>>
function libxml_disable_entity_loader(bool $disable = true): bool;

/* Set the streams context for the next libxml document load or write */
<<__Native>>
function libxml_set_streams_context(resource $context): void;
