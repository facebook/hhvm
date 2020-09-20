<?hh

final class C {
  private darray<string, int> $ints = darray[];

  <<__ALWAYS_INLINE>>
  public function addInt(string $name, int $value): this {
    $this->ints[$name] = $value;
    return $this;
  }

  <<__ALWAYS_INLINE>>
  public function get(): darray<string, int> {
    return __hhvm_intrinsics\launder_value($this->ints);
  }
}

final class O {
  private string $time;

  public function __construct($time) {
    $this->time = __hhvm_intrinsics\launder_value($time);
  }

  <<__ALWAYS_INLINE>>
  public function frob(): C {
    $c = new C();
    $c->addInt('foo', (int)$this->time);
    return $c;
  }
}

<<__EntryPoint>>
function main() {
  $o = new O('29857923');
  $c = $o->frob();
  $d = $c->get();
  serialize($d);
}
