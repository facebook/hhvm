<?hh

function f() {
  $a = array();
  try { echo $a[15410]; } catch (Exception $e) { echo $e->getMessage()."\n"; }
  echo "In f: " . error_reporting() . "\n";
}

function g() {
  $a = array();
  try { echo $a[15411]; } catch (Exception $e) { echo $e->getMessage()."\n"; }
  echo "In f: " . error_reporting() . "\n";
  echo "In g: " . error_reporting() . "\n";
  error_reporting(15251);
}
<<__EntryPoint>>
function main_entry(): void {


  echo error_reporting() . "\n";
  f();
  echo error_reporting() . "\n";
  @f();
  echo error_reporting() . "\n";

  echo error_reporting() . "\n";
  @g();
  echo error_reporting() . "\n";

  $arr = array();
  try { echo @$arr['nope']; } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
