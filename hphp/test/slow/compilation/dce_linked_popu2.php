<?hh
<<__EntryPoint>>
function main(): void {
  var_dump(t(42));
}

function f($k) :mixed{
  return null;
}

function t(HH\arraykey $k) :mixed{
  return f(darray[HH\array_key_cast($k) => $k]);
}
