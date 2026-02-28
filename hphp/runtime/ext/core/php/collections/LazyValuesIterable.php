<?hh

class LazyValuesIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;

  public function __construct($iterable)[] {
    $this->iterable = $iterable;
  }
  public function getIterator()[] {
    return new LazyValuesIterator($this->iterable->getIterator());
  }
}
