<?hh

function f1($x) :mixed{
  return isset($x[0]) && $x[0];
}

function f2($x) :mixed{
  try {
    if (!is_null($x[0])) var_dump($x[0]);
    var_dump($x[0]);
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}

function f3($x) :mixed{
  foreach ($x['foo'] as $k => $v) {
    try {
      if ($v) unset($x['foo'][$k]);
    } catch (Exception $e) { echo $e->getMessage()."\n"; }
  }
  var_dump($x);
}

function f4($x) :mixed{
  try {
    var_dump($x[0][1]);
    unset($x[0][1]);
    var_dump($x[0][1]);
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}

function f5($x) :mixed{
  var_dump(md5($x[0]), $x[0]);
}


<<__EntryPoint>>
function main_537() :mixed{
error_reporting(0);
var_dump(f1(null));
var_dump(f1(vec[]));
var_dump(f1(vec[0]));
var_dump(f1(''));
var_dump(f1('a'));
f2(dict[0 => vec[]]);
f2(vec[]);
f2('');
f2(null);
f3(dict['foo' => vec[0,1,2,3]]);
f4(vec[dict[1 => new stdClass()]]);
f5('foobar');
}
