//// isolated/attr.php
<?hh
// A user attribute class in the strict-isolation package `isolated`.
class IsolatedAttr2 implements HH\FunctionAttribute {
  public function __construct() {}
}

//// isolated/use.php
<?hh
// Same package (`isolated`) may use its own attribute: no violation.
<<IsolatedAttr2>>
function use_within_isolated(): void {}
