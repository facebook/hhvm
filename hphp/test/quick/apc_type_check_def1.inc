class A <reify T as num as arraykey> {
  public function __construct(T $y) {
    $this->x = $y;
  }
  public T $x;
}
