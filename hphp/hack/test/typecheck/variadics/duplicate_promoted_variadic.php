<?hh

class C {
  private vec<int> $args;
  public function __construct(private int... $args) { }
}
