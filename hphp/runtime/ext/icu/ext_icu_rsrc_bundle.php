<?hh

/**
 * Localized software products often require sets of data that are to be
 * customized depending on current locale, e.g.: messages, labels, formatting
 * patterns. ICU resource mechanism allows to define sets of resources that
 * the application can load on locale basis, while accessing them in unified
 * locale-independent fashion.   This class implements access to ICU resource
 * data files. These files are binary data arrays which ICU uses to store the
 * localized data.   ICU resource bundle can hold simple resources and complex
 * resources. Complex resources are containers which can be either
 * integer-indexed or string-indexed (just like PHP arrays). Simple resources
 * can be of the following typos: string, integer, binary data field or
 * integer array.   ResourceBundle supports direct access to the data through
 * array access pattern and iteration via foreach, as well as access via class
 * methods. The result will be PHP value for simple resources and
 * ResourceBundle object for complex ones. All resources are read-only.
 */
<<__NativeData("ResourceBundle")>>
class ResourceBundle implements \Iterator, \ArrayAccess, \Countable {
  /**
   * Create a resource bundle
   *
   * @param string $locale - Locale for which the resources should be
   *   loaded (locale name, e.g. en_CA).
   * @param string $bundlename - The directory where the data is stored
   *   or the name of the .dat file.
   * @param bool $fallback - Whether locale should match exactly or
   *   fallback to parent locale is allowed.
   *
   * @return ResourceBundle - Returns ResourceBundle object or FALSE on
   *   error.
   */
  <<__Native>>
  public function __construct(mixed $locale,
                              mixed $bundlename,
                              bool $fallback = true): void;
  /**
   * Get number of elements in the bundle
   *
   * @return int - Returns number of elements in the bundle.
   */
  <<__Native>>
  public function count(): int;

  /**
   * Create a resource bundle
   *
   * @param string $locale - Locale for which the resources should be
   *   loaded (locale name, e.g. en_CA).
   * @param string $bundlename - The directory where the data is stored
   *   or the name of the .dat file.
   * @param bool $fallback - Whether locale should match exactly or
   *   fallback to parent locale is allowed.
   *
   * @return ResourceBundle - Returns ResourceBundle object or FALSE on
   *   error.
   */
  public static function create(mixed $locale,
                                mixed $bundlename,
                                bool $fallback = false): ResourceBundle {
    try {
      return new ResourceBundle($locale, $bundlename, $fallback);
    } catch (Exception $e) {
      return null;
    }
  }

  /**
   * Get bundle's last error code.
   *
   * @return int - Returns error code from last bundle object call.
   */
  <<__Native>>
  public function getErrorCode(): int;

  /**
   * Get bundle's last error message.
   *
   * @return string - Returns error message from last bundle object's
   *   call.
   */
  <<__Native>>
  public function getErrorMessage(): string;

  /**
   * Get data from the bundle
   *
   * @param string|int $index - Data index, must be string or integer.
   *
   * @return mixed - Returns the data located at the index or NULL on
   *   error. Strings, integers and binary data strings are returned as
   *   corresponding PHP types, integer array is returned as PHP array.
   *   Complex types are returned as ResourceBundle object.
   */
  <<__Native>>
  public function get(mixed $index, bool $fallback = true): mixed;

  /**
   * Get supported locales
   *
   * @param string $bundlename - Path of ResourceBundle for which to get
   *   available locales, or empty string for default locales list.
   *
   * @return array - Returns the list of locales supported by the bundle.
   */
  <<__Native>>
  public static function getLocales(string $bundlename): mixed;

  /* Array Access */

  public function offsetExists(mixed $index): mixed {
    return null !== $this->get($index, true);
  }

  public function offsetGet(mixed $index): mixed {
    return $this->get($index, true);
  }

  public function offsetSet(mixed $index, mixed $value): bool {
    return false;
  }

  public function offsetUnset(mixed $index): bool {
    return false;
  }

  /* Iterator */

  <<__Native>> public function current(): mixed;
  <<__Native>> public function key(): mixed;
  <<__Native>> public function next(): mixed;
  <<__Native>> public function rewind(): mixed;
  <<__Native>> public function valid(): bool;
}

/**
 * Get number of elements in the bundle
 *
 * @param resourcebundle $r - ResourceBundle object.
 *
 * @return int - Returns number of elements in the bundle.
 */
function resourcebundle_count(ResourceBundle $r): int {
  return $r->count();
}

/**
 * Create a resource bundle
 *
 * @param string $locale - Locale for which the resources should be
 *   loaded (locale name, e.g. en_CA).
 * @param string $bundlename - The directory where the data is stored or
 *   the name of the .dat file.
 * @param bool $fallback - Whether locale should match exactly or
 *   fallback to parent locale is allowed.
 *
 * @return ResourceBundle - Returns ResourceBundle object or FALSE on
 *   error.
 */
function resourcebundle_create(mixed $locale,
                               mixed $bundlename,
                               bool $fallback = false): ResourceBundle {
  return ResourceBundle::create($locale, $bundlename, $fallback);
}

/**
 * Get bundle's last error code.
 *
 * @param resourcebundle $r - ResourceBundle object.
 *
 * @return int - Returns error code from last bundle object call.
 */
function resourcebundle_get_error_code(ResourceBundle $r): int {
  return $r->getErrorCode();
}

/**
 * Get bundle's last error message.
 *
 * @param resourcebundle $r - ResourceBundle object.
 *
 * @return string - Returns error message from last bundle object's call.
 */
function resourcebundle_get_error_message(ResourceBundle $r): string {
  return $r->getErrorMessage();
}

/**
 * Get data from the bundle
 *
 * @param resourcebundle $r - ResourceBundle object.
 * @param string|int $index - Data index, must be string or integer.
 *
 * @return mixed - Returns the data located at the index or NULL on
 *   error. Strings, integers and binary data strings are returned as
 *   corresponding PHP types, integer array is returned as PHP array.
 *   Complex types are returned as ResourceBundle object.
 */
function resourcebundle_get(ResourceBundle $r,
                            mixed $index): mixed {
  return $r->get($index);
}

/**
 * Get supported locales
 *
 * @param string $bundlename - Path of ResourceBundle for which to get
 *   available locales, or empty string for default locales list.
 *
 * @return array - Returns the list of locales supported by the bundle.
 */
function resourcebundle_locales(string $bundlename): array {
  return ResourceBundle::getLocales($bundlename);
}
