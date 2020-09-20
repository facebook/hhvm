<?hh

function f1($x) {
  return isset($x[0]) && $x[0];
}

function f2($x) {
  try {
    if (!is_null($x[0])) var_dump($x[0]);
    var_dump($x[0]);
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}

function f3($x) {
  foreach ($x['foo'] as $k => $v) {
    try {
      if ($v) unset($x['foo'][$k]);
    } catch (Exception $e) { echo $e->getMessage()."\n"; }
  }
  var_dump($x);
}

function f4($x) {
  try {
    var_dump($x[0][1]);
    unset($x[0][1]);
    var_dump($x[0][1]);
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}

function f5($x) {
  var_dump(md5($x[0]), $x[0]);
}


<<__EntryPoint>>
function main_537() {
error_reporting(0);
var_dump(f1(null));
var_dump(f1(varray[]));
var_dump(f1(varray[0]));
var_dump(f1(''));
var_dump(f1('a'));
f2(darray[0 => varray[]]);
f2(varray[]);
f2('');
f2(null);
f3(darray['foo' => varray[0,1,2,3]]);
f4(varray[darray[1 => new stdClass()]]);
f5('foobar');
}
