//// unpackaged/override_member.php
<?hh
// Implicit families are unconditionally strictly isolated, so members are too.
<<file: __PackageOverride('prototypes.foo')>>

function test_override_member(): void {}

//// unpackaged/override_family.php
<?hh
// Naming the family itself is likewise refused.
<<file: __PackageOverride('prototypes')>>

function test_override_family(): void {}

//// unpackaged/override_plain.php
<?hh
// A package with no strict isolation is unaffected.
<<file: __PackageOverride('standalone')>>

function test_override_plain(): void {}
