<?hh

class LazySkipKeyedIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable;
  private $n;

  public function __construct($iterable, $n)[] {
    $this->iterable = $iterable;
    $this->n = $n;
  }
  public function getIterator()[] {
    return new LazySkipKeyedIterator($this->iterable->getIterator(),
                                     $this->n);
  }
}
