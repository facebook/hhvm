//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
enum E: int { A = 1; }
class C {}
interface I {}

//// bar.php
<?hh
// package pkg1

// Enum is base-type-enforced, not name-enforced: it errors when non-class-like violations are disallowed, unlike the class/interface which stay carved out.
function param_enum(E $_): void {}
function param_class(C $_): void {}
function param_interface(I $_): void {}
