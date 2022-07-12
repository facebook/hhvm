<?hh

class LazyTakeKeyedIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable;
  private $n;

  public function __construct($iterable, $n)[] {
    $this->iterable = $iterable;
    $this->n = $n;
  }
  public function getIterator()[] {
    return new LazyTakeKeyedIterator($this->iterable->getIterator(),
                                     $this->n);
  }
}
