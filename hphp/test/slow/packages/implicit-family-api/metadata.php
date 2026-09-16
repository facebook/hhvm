<?hh

function metadata_print_exists(string $name): void {
  $exists = HH\implicit_package_family_exists($name) ? 'true' : 'false';
  echo $name.'='.$exists."\n";
}

function metadata_print_families(): void {
  $families = HH\get_all_implicit_package_families();
  \ksort(inout $families);
  foreach ($families as $name => $family) {
    echo $name.': path='.$family['path'].
      '; includes='.\implode(',', $family['includes']).
      '; soft_includes='.\implode(',', $family['soft_includes'])."\n";
  }
}

<<__EntryPoint>>
function metadata_main(): void {
  echo "exists\n";
  foreach (vec['alpha', 'soft', 'zeta', 'alpha.member', 'explicit', 'unknown'] as $name) {
    metadata_print_exists($name);
  }

  echo "families\n";
  metadata_print_families();

  echo "legacy\n";
  var_dump(HH\package_exists('explicit'));
  var_dump(HH\package_exists('alpha'));
  var_dump(\array_key_exists('alpha', HH\get_all_packages()));
}
