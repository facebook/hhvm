<?hh

class C {}

function f<reify T>() {
  var_dump(HH\ReifiedGenerics\getType<T>());
}

<<__EntryPoint>>
function main() {
  f<int>();
  f<C>();
  // make sure hhbbc does not crash here since D is undefined
  @f<D>();
}
