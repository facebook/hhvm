//// isolated/c.php
<?hh
// A class in the strict-isolation package `isolated`.
class IsolatedClassPtr {}

//// intern/use.php
<?hh
// `intern` does not include `isolated`. A `::class` reference to its class is a
// package violation for a strict-isolation package (normally only a classptr
// lint under the migration carve-out).
function use_isolated_classptr(): void {
  $_ = IsolatedClassPtr::class;
}
