<?hh

const E = dict[1 as ~int => 1, "a" as ~string => "a"];
const F = dict[3.14 as ~float => 3.14];

<<__EntryPoint>>
function main(): void {
  var_dump(E);
  var_dump(F);
}
