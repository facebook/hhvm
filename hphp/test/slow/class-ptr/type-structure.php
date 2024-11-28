<?hh

<<file:__EnableUnstableFeatures('class_type')>>

type T = class<C>;
type U = classname<C>;
class C {
  const type T = class<C>;
  const type U = classname<C>;
}

<<__EntryPoint>>
function main(): void {
  var_dump(type_structure('T'));
  var_dump(type_structure('U'));
  var_dump(type_structure('C', 'T'));
  var_dump(type_structure('C', 'U'));
}
