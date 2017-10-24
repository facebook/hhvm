<?hh

function main($cl1) {
  $cl2 = (inout $x) ==> {
    $x = 5;
    return 6;
  };
  $cl3 = function (&$x) {
    $x = 6;
    return 7;
  };

  $x = null;
  var_dump($cl1(inout $x), $x, $cl2(&$x), $x, $cl3(inout $x), $x);
}

main((inout $x) ==> {
  $x = 4;
  return 5;
});
