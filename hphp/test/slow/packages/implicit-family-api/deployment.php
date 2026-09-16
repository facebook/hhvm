<?hh
<<file: __PackageOverride('shared')>>

function deployment_print_exists(string $name): void {
  $exists = HH\implicit_package_family_exists($name) ? 'true' : 'false';
  echo $name.'='.$exists."\n";
}

<<__EntryPoint>>
function deployment_main(): void {
  echo "deployed\n";
  foreach (vec['alpha', 'soft', 'zeta', 'alpha.member', 'explicit'] as $name) {
    deployment_print_exists($name);
  }

  echo "enumerated\n";
  $families = HH\get_all_implicit_package_families();
  \ksort(inout $families);
  foreach ($families as $name => $_) {
    echo $name."\n";
  }
}
