<?hh

function test() {
  if (@$y is ?int) echo "Yes\n";
}


<<__EntryPoint>>
function main_uninit_vs_nullable() {
test();
}
