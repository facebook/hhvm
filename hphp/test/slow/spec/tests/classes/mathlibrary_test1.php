<?hh
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  include_once 'MathLibrary.inc';

  // $m = new MathLibrary;    // can't instantiate a final class

  MathLibrary::sin(2.34);
  MathLibrary::cos(2.34);
  MathLibrary::tan(2.34);
}
