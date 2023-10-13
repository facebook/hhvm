<?hh

async function f(): Awaitable<?float> {
  return 1.0;
}

function g(): (float, float) {
  $a = (float)f();
  $b = (float)f();
  return tuple($a, $b);
}
