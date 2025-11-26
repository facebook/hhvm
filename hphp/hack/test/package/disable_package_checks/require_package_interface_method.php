//// file.php
<?hh

interface I {
  // this is an error if allow_require_package_on_interface_methods is false
  <<__RequirePackage('pkg')>>
  public function foo(): void;
}
class C {
  <<__RequirePackage('pkg')>>
  public function baz(): void {}
}
