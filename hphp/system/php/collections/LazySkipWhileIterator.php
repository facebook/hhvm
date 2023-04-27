<?hh

class LazySkipWhileIterator implements \HH\Iterator {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->impureInit();
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->impureInit();
    $it = $this->it;
    $fn = $this->fn;
    $it->rewind();
    while ($it->valid() && $fn($it->current())) {
      $it->next();
    }
  }
  public function valid() {
    $this->impureInit();
    return $this->it->valid();
  }
  public function next() {
    $this->impureInit();
    $this->it->next();
  }
  public function key() {
    $this->impureInit();
    return $this->it->key();
  }
  public function current() {
    $this->impureInit();
    return $this->it->current();
  }
  <<__Memoize>>
  private function impureInit(): void {
    $it = $this->it;
    $fn = $this->fn;
    while ($it->valid() && $fn($it->current())) {
      $it->next();
    }
  }
}
