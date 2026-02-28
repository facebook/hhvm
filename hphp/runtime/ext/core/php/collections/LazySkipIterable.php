<?hh

class LazySkipIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $n;

  public function __construct($iterable, $n)[] {
    $this->iterable = $iterable;
    $this->n = $n;
  }
  public function getIterator()[] {
    return new LazySkipIterator($this->iterable->getIterator(),
                                $this->n);
  }
}
