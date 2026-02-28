<?hh

/*
 * This is a small test case for handling comparisons on null SSATmps
 * that aren't isConst().
 */
function ar() :mixed{ return null; }
<<__EntryPoint>> function foo(): void {
  $x = ar();
  echo $x != true;
  echo "\n";
  echo $x == true;
  echo "\n";
  echo HH\Lib\Legacy_FIXME\neq($x, false);
  echo "\n";
  echo HH\Lib\Legacy_FIXME\eq($x, false);
  echo "\n";
}
