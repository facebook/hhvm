<?hh // strict
final class Readonly<T> {
    public function __construct(public readonly T $value)[] {}

    public readonly function get()[] : readonly T {
      return readonly $this->value;
    }

    public function set(readonly T $val)[write_props] : void {
      $this->value = $val;
    }


    public static function unwrapDict<Tk as arraykey, Tv>(
      dict<Tk, Readonly<Tv>> $dict,
    ): readonly dict<Tk, Tv> {
      $result = readonly dict<Tk, Tv>[];
      foreach ($dict as $k => $v) {
        $result[$k] = $v->value;
      }
      return $result;
    }
}


class Ref<T> {
  public function __construct(public int $prop){}
}
<<__EntryPoint>>
function test() : void {
  $x = dict["2" => new Readonly(0),"3" => new Readonly(1)];
  $z = readonly Readonly::unwrapDict($x);
}
