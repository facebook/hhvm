<?hh

class LazyFilterIterator implements \HH\Iterator {
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
    $it = $this->it;
    $fn = $this->fn;
    $it->rewind();
    while ($it->valid() && !$fn($it->current())) {
      $it->next();
    }
  }
  public function valid() {
    return $this->it->valid();
  }
  public function next() {
    $it = $this->it;
    $fn = $this->fn;
    $it->next();
    while ($it->valid() && !$fn($it->current())) {
      $it->next();
    }
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return $this->it->current();
  }
}
