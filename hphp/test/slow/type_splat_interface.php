<?hh
<<file: __EnableUnstableFeatures('type_splat', 'open_tuples')>>

interface IBase<TInputs as (mixed...)> {
  public static function foo(... TInputs $inputs): void;
}
interface I
  extends IBase<
    (string, int)
  > {
}

abstract final class C implements I {
  static public function foo(
    string $s,
    int $i,
  ): void {
    var_dump($s);
    var_dump($i);
  }
}

<<__EntryPoint>>
function main():void {
  C::foo("A", 3);
}
