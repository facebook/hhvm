<?hh // partial

/**
 * A "break iterator" is an ICU object that exposes methods for locating
 * boundaries in text (e.g. word or sentence boundaries). The PHP
 * IntlBreakIterator serves as the base class for all types of ICU break
 * iterators. Where extra functionality is available, the intl extension may
 * expose the ICU break iterator with suitable subclasses, such as
 * IntlRuleBasedBreakIterator or IntlCodePointBreaIterator.   This class
 * implements Traversable. Traversing an IntlBreakIterator yields non-negative
 * integer values representing the successive locations of the text
 * boundaries, expressed as UTF-8 code units (byte) counts, taken from the
 * beginning of the text (which has the location 0). The keys yielded by the
 * iterator simply form the sequence of natural numbers {0, 1, 2, ...}.
 */
<<__NativeData>>
class IntlBreakIterator implements \HH\Iterator {
  /**
   * Private constructor for disallowing instantiation
   */
  private function __construct(): void {}

  /**
   * Create break iterator for boundaries of combining character sequences
   *
   *
   * @param string $locale - Text locale or NULL for default_locale
   *
   * @return IntlBreakIterator
   */
  <<__Native>>
  public static function createCharacterInstance(?string $locale = NULL): IntlBreakIterator;

  /**
   * Create break iterator for boundaries of code points
   *
   * @return IntlCodePointIterator
   */
  <<__Native>>
  public static function createCodePointInstance(): IntlCodePointBreakIterator;

  /**
   * Create break iterator for logically possible line breaks
   *
   * @param string $locale - Text locale or NULL for default_locale
   *
   * @return IntlBreakIterator
   */
  <<__Native>>
  public static function createLineInstance(?string $locale = NULL): IntlBreakIterator;

  /**
   * Create break iterator for sentence breaks
   *
   * @param string $locale - Text locale or NULL for default_locale
   *
   * @return IntlBreakIterator
   */
  <<__Native>>
  public static function createSentenceInstance(?string $locale = NULL): IntlBreakIterator;

  /**
   * Create break iterator for title-casing breaks
   *
   * @param string $locale - Text locale or NULL for default_locale
   *
   * @return IntlBreakIterator
   */
  <<__Native>>
  public static function createTitleInstance(?string $locale = NULL): IntlBreakIterator;

  /**
   * Create break iterator for word breaks
   *
   * @param string $locale - Text locale or NULL for default_locale
   *
   * @return IntlBreakIterator
   */
  <<__Native>>
  public static function createWordInstance(?string $locale = NULL): IntlBreakIterator;

  /**
   * Index into numerically keyed iterator
   *
   * @return mixed - Index or FALSE if invalid/unavailable
   */
  <<__Native>>
  public function key(): mixed;

  /**
   * Alias for first()
   *
   * @return int
   */
  public function rewind(): mixed {
    return $this->first();
  }

  /**
   * current() is a valid value
   *
   * @return bool
   */
  public function valid(): bool {
    return $this->key() !== false;
  }

  /**
   * Get index of current position
   *
   * @return int - Position of current element
   */
  <<__Native>>
  public function current(): mixed;

  /**
   * Set position to the first character in the text
   *
   * @return int - Position of first element
   */
  <<__Native>>
  public function first(): mixed;

  /**
   * Advance the iterator to the first boundary following specified offset
   *
   * @param int $offset - Position within the iterator
   *
   * @return int - Position of following element, or FALSE if none
   */
  <<__Native>>
  public function following(int $offset): mixed;

  /**
   * Get last error code on the object
   *
   * @return int
   */
  <<__Native>>
  public function getErrorCode(): int;

  /**
   * Get last error message on the object
   *
   * @return string
   */
  <<__Native>>
  public function getErrorMessage(): string;

  /**
   * Get the locale associated with the object
   *
   * @param int $locale_type -
   *
   * @return string
   */
  <<__Native>>
  public function getLocale(int $locale_type): mixed;

  /**
   * Create iterator for navigating fragments between boundaries
   *
   * @param string $key_type -
   *
   * @return IntlPartsIterator
   */
  <<__Native>>
  public function getPartsIterator(string $key_type): IntlPartsIterator;

  /**
   * Get the text being scanned
   *
   * @return string
   */
  <<__Native>>
  public function getText(): mixed;

  /**
   * Tell whether an offset is a boundary's offset
   *
   * @param int $offset
   *
   * @return bool
   */
  <<__Native>>
  public function isBoundary(int $offset): bool;

  /**
   * Set the iterator position to index beyond the last character
   */
  <<__Native>>
  public function last(): mixed;

  /**
   * Advance the iterator the next boundary
   *
   * @param int $offset
   *
   * @return int
   */
  <<__Native>>
  public function next(?int $offset = NULL): mixed;

  /**
   * Set the iterator position to the first boundary before an offset
   *
   * @param int $offset
   *
   * @return string
   */
  <<__Native>>
  public function preceding(int $offset): mixed;

  /**
   * Set the iterator position to the boundary immediately before the current
   *
   * @return int
   */
  <<__Native>>
  public function previous(): mixed;

  /**
   * Set the text being scanned
   *
   * @param string $text
   *
   * @return bool
   */
  <<__Native>>
  public function setText(string $text): bool;
}

/////////////////////////////////////////////////////////////////////////////

class IntlCodePointBreakIterator extends IntlBreakIterator {
  <<__Native>>
  public function getLastCodePoint(): int;
}

/////////////////////////////////////////////////////////////////////////////

class IntlRuleBasedBreakIterator extends IntlBreakIterator {
  <<__Native>>
  public function __construct(string $rules, bool $compiled = false): void;

  <<__Native>>
  public function getRules(): mixed;

  <<__Native>>
  public function getRuleStatus(): int;

  <<__Native>>
  public function getRuleStatusVec(): mixed;

  <<__Native>>
  public function getBinaryRules(): mixed;
}

/////////////////////////////////////////////////////////////////////////////

class IntlPartsIterator extends IntlIterator {
  <<__Native>>
  public function getBreakIterator(): IntlBreakIterator;
}
