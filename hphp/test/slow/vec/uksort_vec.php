<?hh

function test(bool $throw) :mixed{
  $v = __hhvm_intrinsics\launder_value(vec[1, 2, 3]);
  var_dump($v);
  uksort(inout $v, ($p1, $p2) ==> {
    if ($throw) throw new Exception("boom\n");
    return $p1 <=> $p2;
  });
  var_dump($v);
}

<<__EntryPoint>>
function main() :mixed{
  test(false);
  try {
    test(true);
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
