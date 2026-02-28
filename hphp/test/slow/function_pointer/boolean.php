<?hh

function rfoo<reify T>() {}

<<__EntryPoint>>
function main() {
  var_dump((bool)rfoo<int>);
  var_dump((bool)__hhvm_intrinsics\launder_value(rfoo<int>));
}
