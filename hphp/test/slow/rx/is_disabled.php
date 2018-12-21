<?hh

<<__Rx>>
function react() {
  if (Rx\IS_ENABLED) {
    // can't have side effects
    return 0;
  } else {
    // side effects OK
    echo "disabled\n";
    return 0;
  }
}

<<__EntryPoint>>
function main() {
  var_dump(react());
  echo "Done\n";
}
