<?hh

class LazyKVZipIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;

  public function __construct($iterable)[] {
    $this->iterable = $iterable;
  }
  public function getIterator()[] {
    return new LazyKVZipIterator($this->iterable->getIterator());
  }
}
