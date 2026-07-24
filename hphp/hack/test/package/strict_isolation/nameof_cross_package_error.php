//// isolated/c.php
<?hh
// A class in the strict-isolation package `isolated`.
class IsolatedNameof {}

//// intern/use.php
<?hh
// `intern` does not include `isolated`. Even though `nameof` does not require the
// package to be loaded, a strict-isolation package may not be statically
// referenced from outside, so this is a hard package-boundary error.
function use_isolated_nameof(): void {
  $_ = nameof IsolatedNameof;
}
