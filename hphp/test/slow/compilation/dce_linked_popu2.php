<?hh
<<__EntryPoint>>
function main(): void {
  var_dump(t(42));
}

function f($k) {
  return null;
}

function t(HH\arraykey $k) {
  return f(darray[HH\array_key_cast($k) => $k]);
}
