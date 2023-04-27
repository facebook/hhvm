<?hh

class LazyTakeKeyedIterator implements \HH\KeyedIterator {
  private $it;
  private $n;
  private $numLeft;

  public function __construct($it, $n)[] {
    $this->it = $it;
    $this->n = $n;
    $this->numLeft = $n;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
    $this->numLeft = $this->n;
  }
  public function valid(): bool {
    return ($this->numLeft > 0 && $this->it->valid());
  }
  public function next() {
    $this->it->next();
    --$this->numLeft;
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return $this->it->current();
  }
}
