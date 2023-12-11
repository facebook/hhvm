<?hh

function test($a, $b) :mixed{
  $buf = 'hello';
  foreach ($a as $v) {
    $buf .= $v . ';';
    foreach ($b as $w) {
      $buf .= $w;
    }
  }
  var_dump($buf);
}
function test2($a, $b) :mixed{
  $buf = 'hello';
  foreach ($a as $v) {
    $buf .= $v . ';';
    foreach ($b as $w) {
      $buf .= $w;
    }
    echo $buf;
  }
  var_dump($buf);
}
function test3($a, $b) :mixed{
  $buf = 'hello';
  foreach ($a as $v) {
    $buf .= $v . ';';
    foreach ($b as $w) {
      echo ($buf .= $w);
    }
  }
  var_dump($buf);
}

<<__EntryPoint>>
function main_165() :mixed{
test(vec['a', 'b', 'c'], vec['u', 'v']);
test2(vec['a', 'b', 'c'], vec['u', 'v']);
test3(vec['a', 'b', 'c'], vec['u', 'v']);
}
