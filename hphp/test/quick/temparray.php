<?hh

abstract final class DynstringStatics {
  public static $state = 0x71177beef7337;
}

function dynString() :mixed{
  // Defeat any conceivable constant-folding smarts
  DynstringStatics::$state = (DynstringStatics::$state << 3) ^ 022707;
  return (string)DynstringStatics::$state;
}

function dynArray($n) :mixed{
  $a = vec[];
  foreach (range(0, $n) as $i) {
    $a[] = dynString();
  }
  return $a;
}
<<__EntryPoint>> function main(): void {
var_dump(dynArray(2)[1]);
}
