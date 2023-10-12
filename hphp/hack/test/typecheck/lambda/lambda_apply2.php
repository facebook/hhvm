<?hh // strict

function test(): void {
  apply(
    $c ==> {
      if (false) {
        return $c->get();
      } else {
        return null;
      }
    },
    new C(42),
  );
}

final class C<T> {
  public function __construct(public T $value) {}

  public function get(): T {
    return $this->value;
  }
}

function apply<TA, TB>((function(TA): TB) $f, TA $x): TB {
  return $f($x);
}
