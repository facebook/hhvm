<?hh

class LazyFilterWithKeyIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return
      new LazyFilterWithKeyIterator($this->iterable->getIterator(), $this->fn);
  }
}
