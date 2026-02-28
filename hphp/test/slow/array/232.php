<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
  $a = dict['a' => 'apple', 'b' => 'banana', 'c' => 'citrus'];
  foreach ($a as $k1 => $v1) {
    foreach ($a as $k2 => $v2) {
      if ($k2 == 'a') {
        unset($a[$k2]);
      }
      var_dump($v1, $v2);
    }
  }
  yield null;
}

<<__EntryPoint>>
function main_232() :mixed{
foreach (gen() as $_) {
}
}
