//// isolated/c.php
<?hh
// A class in the strict-isolation package `isolated`.
class IsolatedClassPtr2 {}

//// isolated/use.php
<?hh
// Same package (`isolated`) may take a `::class` reference to its own class.
function use_within_isolated(): void {
  $_ = IsolatedClassPtr2::class;
}
