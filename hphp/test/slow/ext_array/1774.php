<?hh

function cmp($a, $b) {
  throw new Exception('Surprise!');
}
function test() {
  $a = array(1,2,3);
  try {
    usort(&$a, fun('cmp'));
    var_dump('unreached');
  }
 catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1774() {
test();
}
