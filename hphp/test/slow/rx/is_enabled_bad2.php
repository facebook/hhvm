<?hh

function test()[rx] {
  if (Rx\IS_ENABLED) {
    return "FAIL";
  }
  // no else
}

<<__EntryPoint>>
function main() {
  test();
}
