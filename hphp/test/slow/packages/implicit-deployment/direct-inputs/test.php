<?hh

<<file:__PackageOverride('entrypoint')>>

<<__EntryPoint>>
function main(): void {
  implicit_inputs_hard();
  echo "absent: ",
    function_exists('implicit_inputs_absent') ? "present\n" : "missing\n";
}
