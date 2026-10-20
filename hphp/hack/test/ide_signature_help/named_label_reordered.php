<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

interface IFLab3 {}
abstract class FLab3<Ta, Tr> implements IFLab3 {
  public abstract function apply(Ta $_): Tr;
}

class MulLab3 extends FLab3<(int, int), int> {
  public function apply((int, int) $p): int {
    return 0;
  }
}

enum class FuncsLab3: IFLab3 {
  FLab3<(int, int), int> Mul = new MulLab3();
}

function sapp<Ta, Tr>(
  named \HH\EnumClass\Label<FuncsLab3, FLab3<Ta, Tr>> $l,
  named Ta $arg,
): Tr {
  return FuncsLab3::valueOf($l)->apply($arg);
}

<<__EntryPoint>>
function testsapp(): void {
  $r = sapp(arg=tuple(2, 4), l=#Mul);
  //                             ^ at-caret
}
