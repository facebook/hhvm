<?hh

function p(array $i = null) {
  var_dump($i);
  $i = varray[];
}
function q() {
  p(null);
}

<<__EntryPoint>>
function main_1747() {
p();
}
