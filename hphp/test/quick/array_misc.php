<?hh

class X {
}

function test($a) {
  apc_store('foo', array(1 => $a));
  $a = apc_fetch('foo');
  $a[1] = 'bar';
  var_dump($a);

  $a = apc_fetch('foo');
  $a["1"] = 'bar';
  var_dump($a);

  $a = apc_fetch('foo');
  foreach ($a as $k => $x) {
    var_dump($x);
  }

  $a = apc_fetch('foo');
  $a[1] = 'bar';
  foreach ($a as $k => $x) {
    var_dump($x);
  }
}
<<__EntryPoint>> function main() {
test(42);
}
