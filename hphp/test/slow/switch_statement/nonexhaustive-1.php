<?hh

function f($x) {
  switch($x) {
    case 1:
      return 1;
    case 2:
  }
  return 2;
}

<<__EntryPoint>>
function main() {
  var_dump(f(1));
  var_dump(f(2));
  var_dump(f(3));
  var_dump(f(4));
}
