//// isolated/exn.php
<?hh
// An exception class in the strict-isolation package `isolated`.
class IsolatedException extends Exception {}

//// intern/use.php
<?hh
// `intern` does not include `isolated`; catching its exception statically
// references the strict-isolation package, which is a hard package violation.
function use_isolated_catch(): void {
  try {
  } catch (IsolatedException $_e) {
  }
}
