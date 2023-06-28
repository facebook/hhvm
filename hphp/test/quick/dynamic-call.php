<?hh

function f17() :mixed{}

function test($n) :mixed{
  $fn = "f$n";
  $fn();
}

<<__EntryPoint>>
function main_entry() :mixed{
  test(17);
}
