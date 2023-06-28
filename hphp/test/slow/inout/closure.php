<?hh

function main($cl1) :mixed{
  $cl2 = (inout $x) ==> {
    $x = 5;
    return 6;
  };
  $cl3 = function (inout $x) {
    $x = 6;
    return 7;
  };

  $x = null;
  var_dump($cl1(inout $x), $x, $cl2(inout $x), $x, $cl3(inout $x), $x);
}


<<__EntryPoint>>
function main_closure() :mixed{
main((inout $x) ==> {
  $x = 4;
  return 5;
});
}
