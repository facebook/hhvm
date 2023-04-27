<?hh

final class ConstantIterator<T> implements Iterator<T> {
  public function __construct(private T $value) {}
  public function current(): T { return $this->value; }
  public function next(): void {}
  public function rewind(): void {}
  public function valid(): bool { return true; }
}
