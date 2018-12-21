<?hh

<<__Rx>>
function test() {
  if (Rx\IS_ENABLED) {
    return "FAIL";
  }
  // no else
}

<<__EntryPoint>>
function main() {
  test();
}
