<?hh

/* An iterator implementation for iterating over a Set.
 */
<<__NativeData("SetIterator")>>
final class SetIterator implements HH\Iterator {

  public function __construct(): void {}

  /* Returns the current value that the iterator points to.
   * @return mixed
   */
  <<__Native>>
  public function current(): mixed;

  /* @return mixed
   */
  <<__Native>>
  public function key(): mixed;

  /* Returns true if the iterator points to a valid value, returns false
   * otherwise.
   * @return bool
   */
  <<__Native>>
  public function valid(): bool;

  /* Advance this iterator forward one position.
   */
  <<__Native>>
  public function next(): void;

  /* Move this iterator back to the first position.
   */
  <<__Native>>
  public function rewind(): void;
}
