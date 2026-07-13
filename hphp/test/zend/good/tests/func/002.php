<?hh

abstract final class BlahStatics {
  public static $hey =0;
  public static $yo =0;
}
function blah()
:mixed{

  $__hey = BlahStatics::$hey; BlahStatics::$hey++;
  $__yo = BlahStatics::$yo; BlahStatics::$yo--;
  echo "hey=".$__hey.", ",$__yo."\n";
}
<<__EntryPoint>> function main(): void {
blah();
blah();
blah();
if (isset($hey) || isset($yo)) {
  echo "Local variables became global :(\n";
}
}
