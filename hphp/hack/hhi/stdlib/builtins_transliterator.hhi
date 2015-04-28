<?hh // decl

// Based on runtime/ext/icu/ext_icu_transliterator.php

class Transliterator {
  const int FORWARD = 0;
  const int REVERSE = 1;

  public string $id = '';

  private final function __construct();
  private final function __init(string $idOrRules,
                                int $direction,
                                bool $rules): bool;
  public static function create(
    string $id,
    int $direction = self::FORWARD,
  ): ?Transliterator;
  public static function createFromRules(
    string $rules,
    int $direction = self::FORWARD,
  ): ?Transliterator;
  private final function __createInverse(): ?Transliterator;
  public function createInverse(): ?Transliterator;
  public function getErrorCode(): int;
  public function getErrorMessage(): string;
  public function getId();
  public static function listIDs();
  public function transliterate(
    string $subject,
    int $start = 0,
    int $end = -1,
  );
}

function transliterator_create(
  string $id,
  int $direction = Transliterator::FORWARD,
): ?Transliterator;

function transliterator_create_from_rules(
  string $id,
  int $direction = Transliterator::FORWARD,
): ?Transliterator;

function transliterator_create_inverse(Transliterator $t): ?Transliterator;
function transliterator_get_error_code(Transliterator $t): int;
function transliterator_get_error_message(Transliterator $t): string;
function transliterator_list_ids(): array;

function transliterator_transliterate(
  $tOrId,
  string $subject,
  int $start = 0,
  int $end = -1,
);

