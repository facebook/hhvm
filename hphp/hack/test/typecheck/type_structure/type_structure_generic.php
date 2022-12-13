<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface I { }
abstract class C implements I
{
  const type TC = int;
  const type TC2 = C;

  public static function getClassname<T>(TypeStructure<T> $ts): ?classname<T> {
    $y = $ts['classname'];
    return $y;
  }
  public static function getClassname2<T as mixed>(TypeStructure<T> $ts): ?classname<T> {
    $y = $ts['classname'];
    return $y;
  }
  public static function getClassname3<T as I>(TypeStructure<T> $ts): ?classname<T> {
    $y = $ts['classname'];
    return $y;
  }
}
<<__EntryPoint>>
function main():void {
  $x = type_structure(C::class, 'TC');
  $y = type_structure(C::class, 'TC2');
  C::getClassname($x);
  C::getClassname3($y);
}
