<?hh

class LazySliceIterator implements \HH\Iterator {
  private $it;
  private $start;
  private $len;
  private $currentLen;

  public function __construct($it, $start, $len)[] {
    $this->it = $it;
    $this->start = $start;
    $this->len = $len;
    $this->currentLen = $len;
  }
  public function __clone() {
    $this->impureInit();
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->impureInit();
    $it = $this->it;
    $start = $this->start;
    $len = $this->len;
    $it->rewind();
    $this->currentLen = $len;
    while ($start !== 0 && $it->valid()) {
      $it->next();
      --$start;
    }
  }
  public function valid(): bool {
    $this->impureInit();
    return $this->it->valid() && $this->currentLen !== 0;
  }
  public function next() {
    $this->impureInit();
    $this->it->next();
    if ($this->currentLen !== 0) {
      --$this->currentLen;
    }
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
    $start = $this->start;
    while ($start !== 0 && $it->valid()) {
      $it->next();
      --$start;
    }
  }
}
