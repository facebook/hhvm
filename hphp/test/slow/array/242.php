<?hh

function test($q, $a, $b, $c) :mixed{
  $x = dict[0 => $a, 'foo'=> $a];
  if ($x) {
    var_dump(isset($x[0][1]), isset($x['foo'][1]));
    var_dump(isset($x[$b][1]), isset($x[$c][1]));
    $xx = $x[0];
  }
}

<<__EntryPoint>>
function main_242() :mixed{
test(5, vec[0,1], 0, 'foo');
}
