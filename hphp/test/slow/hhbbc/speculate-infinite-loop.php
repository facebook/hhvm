<?hh

function blah() {
  while (1 !== 0) {}
}

<<__EntryPoint>>
function main() {
  if (__hhvm_intrinsics\launder_value(false)) {
    blah();
    echo "not reached\n";
  } else {
    echo "DONE\n";
  }
}
