<?hh

class LazyMapWithKeyIterator implements \HH\KeyedIterator {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
  }
  public function valid() {
    return $this->it->valid();
  }
  public function next() {
    $this->it->next();
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return ($this->fn)($this->it->key(), $this->it->current());
  }
}
