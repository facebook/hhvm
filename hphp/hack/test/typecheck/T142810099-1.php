<?hh

class C<+T> {
  public ?this $self;
  public function __construct(private T $data) {}
  public function get(): T { return $this->data; }
}

<<__EntryPoint>>
function entry(): int {
  $ci = new C<int>(42);
  $cs = new C<string>("hi");
  $ca = new C<arraykey>("hi");
  /* hh_show_env(); */
  $ca->self = $ci;
  ($ca->self as nonnull)->self = $cs;
  return ($ci->self as nonnull)->get();
}
