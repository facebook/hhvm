<?hh

// Based on runtime/ext/icu/ext_icu_transliterator.php

class Transliterator {
  const int FORWARD;
  const int REVERSE;

  public string $id = '';

  private function __construct();
  private function __init(string $idOrRules, int $direction, bool $rules): bool;
  public static function create(
    string $id,
    int $direction = self::FORWARD,
  ): ?Transliterator;
  public static function createFromRules(
    string $rules,
    int $direction = self::FORWARD,
  ): ?Transliterator;
  private function __createInverse(): ?Transliterator;
  public function createInverse(): ?Transliterator;
  public function getErrorCode(): int;
  public function getErrorMessage(): string;
  public function getId(): HH\FIXME\MISSING_RETURN_TYPE;
  public static function listIDs(): HH\FIXME\MISSING_RETURN_TYPE;
  public function transliterate(
    string $subject,
    int $start = 0,
    int $end = -1,
  ): HH\FIXME\MISSING_RETURN_TYPE;
}

<<__PHPStdLib>>
function transliterator_create(
  string $id,
  int $direction = Transliterator::FORWARD,
): ?Transliterator;

<<__PHPStdLib>>
function transliterator_create_from_rules(
  string $id,
  int $direction = Transliterator::FORWARD,
): ?Transliterator;

<<__PHPStdLib>>
function transliterator_create_inverse(Transliterator $t): ?Transliterator;
<<__PHPStdLib>>
function transliterator_get_error_code(Transliterator $t): int;
<<__PHPStdLib>>
function transliterator_get_error_message(Transliterator $t): string;
<<__PHPStdLib>>
function transliterator_list_ids(): mixed;

<<__PHPStdLib>>
function transliterator_transliterate(
  HH\FIXME\MISSING_PARAM_TYPE $tOrId,
  string $subject,
  int $start = 0,
  int $end = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
