<?hh

<<file:__EnableUnstableFeatures('case_types')>>

case type C = int | vec<T>;
type T = (C, C);

<<__EntryPoint>>
function main() {
  var_dump(HH\type_structure('C'));
  var_dump(HH\type_structure('T'));
}
