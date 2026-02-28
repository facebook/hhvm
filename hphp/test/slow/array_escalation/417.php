<?hh

function test() :mixed{
  $a = vec[];
  for ($i = 0;
 $i < 17;
 $i++) {
    $a[] = $i;
  }
  unset($a[16]);
  $b = $a;
  array_unshift(inout $a, 'foo');
  var_dump(count($a), count($b));
}

<<__EntryPoint>>
function main_417() :mixed{
test();
}
