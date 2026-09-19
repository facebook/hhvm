//// host/h.php
<?hh <<file: __PackageOverride('unobservable')>>
// Omitting `allow_deployed_packages_checking` forbids checking for the package,
// not joining it. `enable_strict_isolation` would forbid both.
function in_unobservable(): void {}
