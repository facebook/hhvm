<?hh

<<__Rx>>
function test() {
  if (Rx\IS_ENABLED) {
    return 0;
  } else {
    echo "disabled\n";
    return 0;
  }
  echo "uh-oh!\n";
}

<<__EntryPoint>>
function main() {
  var_dump(test());
}
