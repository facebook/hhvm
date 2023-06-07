<?hh

class LazyKeysIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;

  public function __construct($iterable)[] {
    $this->iterable = $iterable;
  }
  public function getIterator()[] {
    return new LazyKeysIterator($this->iterable->getIterator());
  }
}
