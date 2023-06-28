<?hh

class F {
  public $foo;
}

function t($o, $memb) :mixed{
  var_dump($o->$memb);
  unset($o->$memb);
  try {
    var_dump($o->$memb);
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }
}

function u() :mixed{
  echo "------------------------\n";
  $obj = new F;
  try {
    $obj->foo = $x;
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  foreach ($obj as $k => $_) {
    echo $k."\n";
  }
  echo "------------------------\n";
  $obj = new F;
  try {
    $obj->foo = $y++;
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  foreach ($obj as $k => $_) {
    echo $k."\n";
  }
}

function main() :mixed{
  print "Test begin\n";

  $f = new F();
  $f->foo = 12;
  $f->bart = "snoot";
  var_dump($f);

  t($f, 'foo');
  t($f, 'bart');
  var_dump($f);

  $e = error_reporting(0);
  u();

  error_reporting($e);
  print "Test end\n";
}


function getprop($o) :mixed{
  try {
    return $o->declprop;
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }
}
function setprop($o, $v) :mixed{
  $o->declprop = $v;
}
class c2 {
  public $declprop = 'blah';
}

function main2() :mixed{
  $o = new c2();
  setprop($o, 'set1');
  var_dump(getprop($o));
  unset($o->declprop);
  var_dump(getprop($o));
  setprop($o, 'set2');
  var_dump(getprop($o));
}
<<__EntryPoint>>
function main_entry(): void {
  main();
  main2();
}
