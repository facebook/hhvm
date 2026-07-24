//// standalone/c.php
<?hh
// A class in `standalone`, which does NOT opt into strict isolation.
class StandaloneNameof {}

//// intern/use.php
<?hh
// `intern` does not include `standalone`, but `standalone` is not a
// strict-isolation package, so `nameof` on it produces no warning.
function use_standalone_nameof(): void {
  $_ = nameof StandaloneNameof;
}
