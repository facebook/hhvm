<?hh

<<__SupportDynamicType>>
class Box<T> {
  public function __construct(private T $x) {}
  public function set(T $y): void {
    $this->x = $y;
  }
}
