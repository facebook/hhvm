<?hh

<<file:__EnableUnstableFeatures('class_type')>>

type T = class<C>;
type U = classname<C>;
type V = class_or_classname<C>;
class C {
  const type T = class<C>;
  const type U = classname<C>;
  const type V = class_or_classname<C>;
}

<<__EntryPoint>>
function main(): void {
  var_dump(type_structure('T'));
  var_dump(type_structure('U'));
  var_dump(type_structure('V'));
  var_dump(type_structure('C', 'T'));
  var_dump(type_structure('C', 'U'));
  var_dump(type_structure('C', 'V'));
}
