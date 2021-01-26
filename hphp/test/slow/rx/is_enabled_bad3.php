<?hh

function test()[rx] {
  if (!Rx\IS_ENABLED) {
    echo "disabled\n";
    return 0;
  } else {
    return 0;
  }
}

<<__EntryPoint>>
function main() {
  var_dump(test());
}
