<?hh

class Box<T> {
  public function __construct(private T $t) {}
  public function get(): T {
    return $this->t;
  }
  public function set(T $t): void {
    $this->t = $t;
  } 
}

function bad(Box<int> $bi, Box<arraykey> $ba): void {
  if ($bi == $ba) {
    $bi->set('str');
  }
}

function typed(Box<int> $bi, Box<arraykey> $ba): int {
  bad($bi, $ba);
  return $bi->get();
}

<<__EntryPoint>>
function main(): void {
  $bi = new Box<int>(1);
  $ba = new Box<arraykey>(1);
  typed($bi, $ba);
}
