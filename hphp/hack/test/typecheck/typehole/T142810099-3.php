<?hh

class D {
  private ?this $self;
  public function set_self(this $x): void { $this->self = $x; }
  public function get_self(): this { return $this->self as nonnull; }
}

class C<+T> extends D {
  public function __construct(private T $data) {}
  public function get(): T { return $this->data; }
}

<<__EntryPoint>>
function entry(): int {
  $ci = new C<int>(42);
  $cs = new C<string>("hi");
  $ca = new C<arraykey>("hi");
  /* hh_show_env(); */
  $ca->set_self($ci);
  $ca->get_self()->set_self($cs);
  return $ci->get_self()->get();
}
