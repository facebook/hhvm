<?hh


function testit(mixed $m):int {
  $y = HH\FIXME\UNSAFE_CAST<mixed,vec<_>>($m);
  $x = $y[0];
  $a = HH\FIXME\UNSAFE_CAST<mixed,num>($x);
  return $a;
}

<<__EntryPoint>>
function main():void {
  $r = testit(vec["A"]);
  var_dump($r);
}
