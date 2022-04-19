<?hh
class Ref<T> {
  public function __construct(public T $value) {}
}
function get_readonly(): readonly Ref<mixed> {
  return new Ref(0);
}

function mutates_ref(Ref<mixed> $r): int {
  $r->value = 99;
  return $r->value;
}

<<__EntryPoint>>
  function mymain(): void {
      $r = readonly get_readonly();
      $r |> (new Ref(0) |> mutates_ref($$)); // ok
      ($r |> new Ref(0)) |> mutates_ref($$); // ok
      $r |> mutates_ref($$) |> new Ref(0); // not ok
  }
