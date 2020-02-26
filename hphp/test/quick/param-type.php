<?hh


class Bobble {
  public static function frob(string $thing) {
    return 'get';
  }
}
class BobbleProvider {
  private $type = 0;

  public function __call(string $type, $args = varray[]) {
    $name = Bobble::frob($this->type);
    if ($name !== $type) throw new Exception();
    return 0;
  }
}
<<__EntryPoint>> function main(): void {
$o = new BobbleProvider();
$o->wub('hello');
}
