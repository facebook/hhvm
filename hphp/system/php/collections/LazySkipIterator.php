<?hh

class LazySkipIterator implements \HH\Iterator {
  private $it;
  private $n;

  public function __construct($it, $n)[] {
    $this->it = $it;
    $this->n = $n;
  }
  public function __clone() {
    $this->impureInit();
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->impureInit();
    $it = $this->it;
    $n = $this->n;
    $it->rewind();
    while ($n > 0 && $it->valid()) {
      $it->next();
      --$n;
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
    $n = $this->n;
    while ($n > 0 && $it->valid()) {
      $it->next();
      --$n;
    }
  }
}
