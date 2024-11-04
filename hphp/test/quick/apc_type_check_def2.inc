class A<reify T as num as string> {
  public function __construct(T $y) {
    $this->x = $y;
  }
  public T $x;
}
