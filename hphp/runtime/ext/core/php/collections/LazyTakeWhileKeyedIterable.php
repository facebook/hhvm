<?hh

class LazyTakeWhileKeyedIterable<+Tv> implements \HH\KeyedIterable<Tk, Tv> {
  use LazyKeyedIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return new LazyTakeWhileKeyedIterator($this->iterable->getIterator(),
                                          $this->fn);
  }
}
