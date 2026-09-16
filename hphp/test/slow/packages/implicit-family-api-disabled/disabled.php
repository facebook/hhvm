<?hh

<<__EntryPoint>>
function main(): void {
  var_dump(HH\implicit_package_family_exists('alpha'));
  var_dump(HH\get_all_implicit_package_families());
}
