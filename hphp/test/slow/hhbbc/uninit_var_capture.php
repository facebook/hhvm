<?hh

function test() {
  $foo = () ==> $x;
  $x = 1;
  $foo();
}


<<__EntryPoint>>
function main_uninit_var_capture() {
test();
}
