<?hh

class Derp {
  public static function foo(): void {}
  public function bar(): void {}
}

<<__EntryPoint>>
function dvarr_classmeth(): void {
  class_meth(Derp::class, 'foo');
  inst_meth(new Derp(), 'bar');
  echo "success";
}
