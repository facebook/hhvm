<?hh

class LazyConcatIterator implements \HH\Iterator {
  private $it1;
  private $it2;
  private $currentIt;
  private $state;

  public function __construct($it1, $it2)[] {
    $this->it1 = $it1;
    $this->it2 = $it2;
  }
  public function __clone() {
    $this->impureInit();
    $this->it1 = clone $this->it1;
    $this->it2 = clone $this->it2;
    $this->currentIt = ($this->state === 1) ? $this->it1 : $this->it2;
  }
  public function rewind() {
    $this->impureInit();
    $this->it1->rewind();
    $this->it2->rewind();
    $this->currentIt = $this->it1;
    $this->state = 1;
    if (!$this->currentIt->valid()) {
      $this->currentIt = $this->it2;
      $this->state = 2;
    }
  }
  public function valid() {
    $this->impureInit();
    return $this->currentIt->valid();
  }
  public function next() {
    $this->impureInit();
    $this->currentIt->next();
    if ($this->state === 1 && !$this->currentIt->valid()) {
      $this->currentIt = $this->it2;
      $this->state = 2;
    }
  }
  public function key() {
    $this->impureInit();
    return $this->currentIt->key();
  }
  public function current() {
    $this->impureInit();
    return $this->currentIt->current();
  }
  <<__Memoize>>
  private function impureInit(): void {
    $this->currentIt = $this->it1;
    $this->state = 1;
    if (!$this->currentIt->valid()) {
      $this->currentIt = $this->it2;
      $this->state = 2;
    }
  }
}
