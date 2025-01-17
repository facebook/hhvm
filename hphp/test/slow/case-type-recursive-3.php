<?hh

<<file:__EnableUnstableFeatures('case_types')>>

case type C<Tk> = int | dict<Tk, T>;
type T = (C<int>, C<string>);

<<__EntryPoint>>
function main() {
  var_dump(HH\type_structure('C'));
  var_dump(HH\type_structure('T'));
}

