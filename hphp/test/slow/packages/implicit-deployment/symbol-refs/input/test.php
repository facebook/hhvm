<?hh

<<file:__PackageOverride('entrypoint')>>

<<__EntryPoint>>
function main(): void {
  implicit_symbol_ref_retained();
  echo "unreferenced: ",
    function_exists('implicit_symbol_ref_unreferenced')
      ? "present\n"
      : "missing\n";
}
