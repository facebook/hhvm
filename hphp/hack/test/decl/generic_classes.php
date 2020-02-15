<?hh

class Bag<T> {
  private T $data;

  public function __construct(T $data): void {
    $this->data = $data;
  }

  public function get(): T {
    return $this->data;
  }
}
