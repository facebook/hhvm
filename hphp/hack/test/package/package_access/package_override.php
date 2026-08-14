//// pkg6/allowed_demote.php
<?hh
// pkg6 includes pkg1, so overriding a pkg6 file down into pkg1 is allowed
<<file: __PackageOverride('pkg1')>>
function pkg_override_allowed_demote(): void {}

//// pkg2/allowed_soft.php
<?hh
// pkg2 soft-includes pkg2_soft, which counts as a dependency, so this is allowed
<<file: __PackageOverride('pkg2_soft')>>
function pkg_override_allowed_soft(): void {}

//// into_including.php
<?hh
// pkg1 does not include pkg2 (pkg2 includes pkg1), so this override is invalid
<<file: __PackageOverride('pkg2')>>
function pkg_override_into_including(): void {}

//// unrelated.php
<?hh
// pkg1 does not include pkg4 (they are unrelated), so this override is invalid
<<file: __PackageOverride('pkg4')>>
function pkg_override_unrelated(): void {}

//// redundant.php
<?hh
// overriding into the file's own path-derived package is a no-op
<<file: __PackageOverride('pkg1')>>
function pkg_override_redundant(): void {}

//// multidef.php
<?hh
// the file attribute is copied onto every definition; the error must be
// reported exactly once, not once per definition
<<file: __PackageOverride('pkg4')>>
function pkg_override_multidef1(): void {}
function pkg_override_multidef2(): void {}
class PkgOverrideMultidef {}
