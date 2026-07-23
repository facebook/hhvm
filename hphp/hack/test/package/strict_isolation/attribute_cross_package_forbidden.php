//// isolated/attr.php
<?hh
// A user attribute class defined in the strict-isolation package `isolated`.
class IsolatedAttr implements HH\FunctionAttribute {
  public function __construct() {}
}

//// intern/use.php
<?hh
// `intern` does not include `isolated`, so referencing its attribute is a
// package violation for the strict-isolation package.
<<IsolatedAttr>>
function use_isolated_attr(): void {}
