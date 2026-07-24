//// isolated/exn.php
<?hh
// An exception class in the strict-isolation package `isolated`.
class IsolatedException2 extends Exception {}

//// isolated/use.php
<?hh
// Same package (`isolated`) may catch its own exception: no violation.
function use_within_isolated(): void {
  try {
  } catch (IsolatedException2 $_e) {
  }
}
