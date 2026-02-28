<?hh

<<file:__PackageOverride('softprod')>>

function qux(): void {
  echo "I am soft-removed from prod\n";
}
