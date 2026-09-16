<?hh

<<file:__PackageOverride('override_in')>>

<<__EntryPoint>>
function main(): void {
  implicit_directory_hard_alpha();
  implicit_directory_hard_beta();
  implicit_directory_soft();
  echo "absent: ",
    function_exists('implicit_directory_absent') ? "present\n" : "missing\n";
  echo "soft sibling: ",
    function_exists('implicit_directory_soft_sibling')
      ? "present\n"
      : "missing\n";
  echo "override-out: ",
    function_exists('implicit_directory_override_out')
      ? "present\n"
      : "missing\n";
  implicit_directory_override_in();
}
