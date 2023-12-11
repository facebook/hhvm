<?hh

function f1($x) :mixed{
  if (count($x) > 0) {
    var_dump($x);
  } else if (count($x[0]) > 0) {
    var_dump($x[0]);
  }
}

function id($x) :mixed{
  return $x;
}
function f2($x) :mixed{
  try { if ($x[0]) var_dump(id($x), $x[0]);
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}

function f3($x) :mixed{
  var_dump($x[0].'/'. $x[1]);
  var_dump($x[0].'/'. $x[1]);
}

function f4($x) :mixed{
  $z = @id($x[0]);
  var_dump($z);
  var_dump($x[0]);
}

<<__EntryPoint>>
function main_538() :mixed{
f1(vec[vec[0, 1, 2]]);
f1('abc');
f2(null);
f2(vec[]);
f2(vec[10]);
f3(vec['first', 'second']);
f3('AB');
f4(vec['e1', 'e2']);
}
