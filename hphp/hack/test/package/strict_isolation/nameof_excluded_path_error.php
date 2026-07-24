//// isolated/__tests__/helper.php
<?hh
// A class on an excluded path (__tests__) within the strict-isolation package
// `isolated`.
class IsolatedTestOnlyNameof {}

//// isolated/use.php
<?hh
// Non-excluded code in `isolated` referencing its own excluded-path class via
// `nameof` is an error too: the class is deployment-only (an excluded path), so
// the reference is forbidden even within the same package. This exercises the
// `ExcludedPathAccess` branch of the nameof check.
function use_isolated_excluded_nameof(): void {
  $_ = nameof IsolatedTestOnlyNameof;
}
