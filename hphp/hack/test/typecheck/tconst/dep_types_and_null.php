<?hh

class Ref<T> {
  public function __construct(public T $value) {}
  public function set(T $value): void {}
  public function get(): T {
    return $this->value;
  }
}

function refstr(Ref<?string> $ref): void {}

function test(): void {
  $o = new Ref(null);
  refstr($o);
  $o->set('');
}

function test2(Exception $e): Ref<?Exception> {
  $o = new Ref(null);
  $o->set(new Exception(''));
  $o->set($e);
  return $o;
}
