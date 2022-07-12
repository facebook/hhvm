<?hh

class LazySliceIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $start;
  private $len;

  public function __construct($iterable, $start, $len)[] {
    $this->iterable = $iterable;
    $this->start = $start;
    $this->len = $len;
  }
  public function getIterator()[] {
    return new LazySliceIterator($this->iterable->getIterator(),
                                 $this->start,
                                 $this->len);
  }
}
