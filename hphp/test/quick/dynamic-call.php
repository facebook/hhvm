<?hh

function f17() {}

function test($n) {
  $fn = "f$n";
  $fn();
}

<<__EntryPoint>>
function main_entry() {
  test(17);
}
