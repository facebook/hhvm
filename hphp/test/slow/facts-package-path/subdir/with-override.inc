<?hh
<<file: __PackageOverride('custom')>>
// In subdir/ with __PackageOverride — should return "custom", not "a".
class WithOverride {}
