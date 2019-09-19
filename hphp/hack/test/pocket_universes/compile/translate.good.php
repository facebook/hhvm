<?hh // strict

class C {
  private int $x;

  public function __construct(int $x) {
    $this->$x = $x;
  }

  public function get() : int {
    return $this->x;
  }

  enum Field {
    case type T0;
    case type T1;
    case string name;
    case T0 default_value;
    case T1 stuff;

    :@A (
      type T0 = int,
      type T1 = string,
      name = "A",
      default_value = 42,
      stuff = "stuff" );

    :@B (
      type T0 = string,
      type T1 = int,
      name = "B",
      default_value = "default_value",
      stuff = 1664 );
  }
}
