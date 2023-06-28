<?hh

function f(darray $a = null, $e) :mixed{
  if (is_int($e) || is_string($e)) {
    $a[$e] ??= darray[];
    $a[$e]['foo'] = 30;
    try { $x = $a[$e]['baz']; } catch (Exception $e) { echo $e->getMessage()."\n"; $x = null; }
  } else {
    $a[$e] = 30;
    try { $x = $a[$e]; } catch (Exception $e) { echo $e->getMessage()."\n"; $x = null; }
    $a[$e] = 50;
  }
  var_dump($a, $x);
}
function g($x, $y) :mixed{
  $x[$y]['foo'] = 30;
  $x[$y]['bar'] = 30;
  $z = $x[$y]['bar'];
  $x[$y]['baz'] = 30;
  if ($z) {
    $x[$y]['baz'] = 30;
  }
  return $x;
}
function h($x, $y) :mixed{
  if ($x) {
    $x[$y]['foo'] = 30;
    $x[$y]['bar'] = 30;
  }
  try { var_dump($x[$y]['foo']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}

<<__EntryPoint>>
function main_534() :mixed{
f(darray[], 'e');
f(darray['e' => darray['baz' => 40]], 'e');
var_dump(f(darray['y' => darray[]], 'y'));
var_dump(f(darray[], 'y'));
h(darray[], 0);
h(darray[0 => darray[]], 0);
}
