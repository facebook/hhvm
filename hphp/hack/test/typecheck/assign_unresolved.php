<?hh

class Ref<T> {
  public function __construct(public T $value) {}
}

function test(bool $b): arraykey {
  if ($b) {
    $x = 42;
  } else {
    $x = 'foo';
  }
  $r = new Ref(3.14);
  $r->value = $x;
  return $x;
}
