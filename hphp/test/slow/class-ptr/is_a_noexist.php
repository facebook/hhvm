<?hh

class A {}

<<__EntryPoint>>
function main(): void {
  var_dump(is_a("NoExist", nameof A, true));
  var_dump(is_a("NoExist", nameof A, false));
  var_dump(is_a(nameof A, "NoExist", true));
  var_dump(is_a(nameof A, "NoExist", false));
  var_dump(is_a("NoExist1", "NoExist2", true));
  var_dump(is_a("NoExist1", "NoExist2", false));
}
