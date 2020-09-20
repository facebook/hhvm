<?hh

function main() {
  $d = dict['foo' => 1];
  foo($d['foo']);
}


<<__EntryPoint>>
function main_hack_array_fpass() {
if (isset($g)) {
  include 'hack-array-fpass1.inc';
} else {
  include 'hack-array-fpass2.inc';
}

main();
}
