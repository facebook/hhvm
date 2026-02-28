<?hh

<<file:__EnableUnstableFeatures('case_types')>>

case type C = string | shape('x' => ?T);
type T = C;

<<__EntryPoint>>
function main() {
  var_dump(HH\type_structure('C'));
  var_dump(HH\type_structure('T'));
}
