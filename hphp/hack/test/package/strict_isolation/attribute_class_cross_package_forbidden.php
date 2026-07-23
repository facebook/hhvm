//// isolated/cattr.php
<?hh
// A class attribute in the strict-isolation package `isolated`.
class IsolatedClassAttr implements HH\ClassAttribute {
  public function __construct() {}
}

//// intern/c.php
<?hh
// `intern` does not include `isolated`; using its class attribute is a violation.
<<IsolatedClassAttr>>
class UsesIsolatedClassAttr {}
