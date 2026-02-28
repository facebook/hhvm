<?hh

<<file: __EnableUnstableFeatures('case_types')>>

case type CThunk = int | (function(): int);

class Ref<T> {
  public function __construct(public T $value) {}
  public function get(): T {
    return $this->value;
  }
  public function set(T $value): void {
    $this->value = $value;
  }
}

function eval_thunk(Ref<CThunk> $thunk): int {
  $c = $thunk->get();
  if ($c is int) {
    return $c;
  } else {
    $res = $c();
    $thunk->set($res);
    return $res;
  }
}
