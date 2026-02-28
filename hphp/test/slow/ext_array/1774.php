<?hh

function cmp($a, $b) :mixed{
  throw new Exception('Surprise!');
}
function test() :mixed{
  $a = vec[1,2,3];
  try {
    usort(inout $a, cmp<>);
    var_dump('unreached');
  }
 catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1774() :mixed{
test();
}
