<?hh

abstract final class BarStatics {
  public static $x = 1;
}

function bar($n) {
  return str_repeat("x", $n) . BarStatics::$x++;
}

function foo() {
  apc_store("foo", bar(50));
  $x = __hhvm_intrinsics\apc_fetch_no_check("foo");
  $x[5] = 1;
  $x = bar(20);
  var_dump($x);
}


<<__EntryPoint>>
function main_string_escalate_bug() {
foo();
}
