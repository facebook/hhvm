<?hh

function test($a, $b) {
  $buf = 'hello';
  foreach ($a as $v) {
    $buf .= $v . ';';
    foreach ($b as $w) {
      $buf .= $w;
    }
  }
  var_dump($buf);
}
function test2($a, $b) {
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
function test3($a, $b) {
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
function main_165() {
test(varray['a', 'b', 'c'], varray['u', 'v']);
test2(varray['a', 'b', 'c'], varray['u', 'v']);
test3(varray['a', 'b', 'c'], varray['u', 'v']);
}
