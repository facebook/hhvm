//// isolated/c.php
<?hh
// A class in the strict-isolation package `isolated`.
class IsolatedNameof2 {}

//// isolated/use.php
<?hh
// Same package (`isolated`): `nameof` on its own class is fine, no warning.
function use_within_isolated(): void {
  $_ = nameof IsolatedNameof2;
}
