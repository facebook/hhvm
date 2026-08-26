//// unpackaged/override_isolated.php
<?hh
// A file cannot join a strict-isolation package by attribute.
<<file: __PackageOverride('isolated')>>

function test_override_isolated(): void {}

//// unpackaged/override_plain.php
<?hh
// Overriding into a package without strict isolation is still allowed.
<<file: __PackageOverride('standalone')>>

function test_override_plain(): void {}

//// unpackaged/override_typedef_only.php
<?hh
// A typedef carries no package membership of its own; still reported.
<<file: __PackageOverride('isolated')>>

type TOverride = int;

//// unpackaged/override_two_defs.php
<?hh
// One attribute, one error, however many definitions it covers.
<<file: __PackageOverride('isolated')>>

function test_override_first(): void {}
function test_override_second(): void {}
