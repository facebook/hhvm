<?hh

/**
 * This class represents iterator objects throughout the intl extension
 * whenever the iterator cannot be identified with any other object provided
 * by the extension. The distinct iterator object used internally by the
 * foreach construct can only be obtained (in the relevant part here) from
 * objects, so objects of this class serve the purpose of providing the hook
 * through which this internal object can be obtained. As a convenience, this
 * class also implements the Iterator interface, allowing the collection of
 * values to be navigated using the methods defined in that interface. Both
 * these methods and the internal iterator objects provided to foreach are
 * backed by the same state (e.g. the position of the iterator and its current
 * value).   Subclasses may provide richer functionality.
 */
<<__NativeData>>
class IntlIterator implements Iterator {

  /**
   * Only for internal use
   */
  private function __construct(): void {}

  /**
   * Get the current element
   *
   * @return ReturnType -
   */
  <<__Native>>
  public function current(): mixed;

  /**
   * Get the current key
   *
   * @return ReturnType -
   */
  <<__Native>>
  public function key(): mixed;

  /**
   * Move forward to the next element
   *
   * @return ReturnType -
   */
  <<__Native>>
  public function next(): mixed;

  /**
   * Rewind the iterator to the first element
   *
   * @return ReturnType -
   */
  <<__Native>>
  public function rewind(): mixed;

  /**
   * Check if current position is valid
   *
   * @return ReturnType -
   */
  <<__Native>>
  public function valid(): bool;
}
