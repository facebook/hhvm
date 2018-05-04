<?hh // strict

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
  hh_show($o);
  refstr($o);
  hh_show($o);
  hh_show($o->get());
  $o->set('');
}

function test2(Exception $e): Ref<?Exception> {
  $o = new Ref(null);
  hh_show($o);
  hh_show($o->get());
  $o->set(new Exception(''));
  $o->set($e);
  hh_show($o);
  return $o;
}
