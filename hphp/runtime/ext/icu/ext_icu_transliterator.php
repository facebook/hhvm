<?hh

/**
 * Transliterator provides transliteration of strings.
 */
<<__NativeData("Transliterator")>>
class Transliterator {
  public string $id = '';

  /**
   * This class may only be instantiated by ::create or ::createFromRules
   */
  private final function __construct() {}

  /**
   * Setup internal transliterator object
   */
  <<__Native>>
  private final function __init(string $idOrRules,
                                int $direction,
                                bool $rules): bool;

  /**
   * Create a transliterator
   *
   * @param string $id -
   * @param int $direction -
   *
   * @return Transliterator - Returns a Transliterator object on success,
   *   or NULL on failure.
   */
  public static function create(string $id,
                                int $direction = self::FORWARD):
                                                              ?Transliterator {
    $obj = new Transliterator();
    if (!$obj->__init($id, $direction, false)) {
      return null;
    }
    $obj->id = (string)$obj->getId();
    return $obj;
  }

  /**
   * Create transliterator from rules
   *
   * @param string $rules -
   * @param string $direction -
   *
   * @return Transliterator - Returns a Transliterator object on success,
   *   or NULL on failure.
   */
  public static function createFromRules(string $rules,
                                         int $direction = self::FORWARD):
                                                              ?Transliterator {
    $obj = new Transliterator;
    if (!$obj->__init($rules, $direction, true)) {
      return null;
    }
    $obj->id = (string)$obj->getId();
    return $obj;
  }

  /**
   * Helper for ->createInverse()
   */
  <<__Native>>
  private final function __createInverse(): ?Transliterator;

  /**
   * Create an inverse transliterator
   *
   * @return Transliterator - Returns a Transliterator object on success,
   *   or NULL on failure
   */
  public function createInverse(): ?Transliterator {
    $obj = $this->__createInverse();
    if (!$obj) {
      return null;
    }
    $obj->id = (string)$obj->getId();
    return $obj;
  }

  /**
   * Get last error code
   *
   * @return int - The error code on success, or FALSE if none exists, or
   *   on failure.
   */
  <<__Native>>
  public function getErrorCode(): int;

  /**
   * Get last error message
   *
   * @return string - The error code on success, or FALSE if none exists,
   *   or on failure.
   */
  <<__Native>>
  public function getErrorMessage(): string;

  /**
   * Fetch the transliterator's ID
   */
  <<__Native>>
  public function getId(): mixed;

  /**
   * Get transliterator IDs
   *
   * @return array - An array of registered transliterator IDs on
   *   success, .
   */
  <<__Native>>
  public static function listIDs(): mixed;

  /**
   * Transliterate a string
   *
   * @param string $subject -
   * @param int $start -
   * @param int $end -
   *
   * @return string - The transfomed string on success, .
   */
  <<__Native>>
  public function transliterate(string $subject,
                                int $start = 0,
                                int $end = -1): mixed;

}

/**
 * Create a transliterator
 *
 * @param string $id -
 * @param int $direction -
 *
 * @return Transliterator - Returns a Transliterator object on success,
 *   or NULL on failure.
 */
function transliterator_create
       (string $id, int $direction = Transliterator::FORWARD): ?Transliterator {
  return Transliterator::create($id, $direction);
}

/**
 * Create transliterator from rules
 *
 * @param string $id -
 * @param int $direction -
 *
 * @return Transliterator - Returns a Transliterator object on success,
 *   or NULL on failure.
 */
function transliterator_create_from_rules
       (string $id, int $direction = Transliterator::FORWARD): ?Transliterator {
  return Transliterator::createFromRules($id, $direction);
}

/**
 * Create an inverse transliterator
 *
 * @return Transliterator - Returns a Transliterator object on success,
 *   or NULL on failure
 */
function transliterator_create_inverse(Transliterator $t): ?Transliterator {
  return $t->createInverse();
}

/**
 * Get last error code
 *
 * @return int - The error code on success, or FALSE if none exists, or
 *   on failure.
 */
function transliterator_get_error_code(Transliterator $t): int {
  return $t->getErrorCode();
}

/**
 * Get last error message
 *
 * @return string - The error code on success, or FALSE if none exists,
 *   or on failure.
 */
function transliterator_get_error_message(Transliterator $t): string {
  return $t->getErrorMessage();
}

/**
 * Get transliterator IDs
 *
 * @return array - An array of registered transliterator IDs on success,
 *   .
 */
function transliterator_list_ids(): array {
  return Transliterator::listIDs();
}

/**
 * Transliterate a string
 *
 * @param mixed $transliterator -
 * @param string $subject -
 * @param int $start -
 * @param int $end -
 *
 * @return  - The transfomed string on success, .
 */
function transliterator_transliterate(mixed $tOrId,
                                      string $subject,
                                      int $start = 0,
                                      int $end = -1): mixed {
  if (!($tOrId instanceof Transliterator)) {
    $tOrId = Transliterator::create((string)$tOrId,
                                    Transliterator::FORWARD);
    if (!$tOrId) {
      return false;
    }
  }
  return $tOrId->transliterate($subject, $start, $end);
}

