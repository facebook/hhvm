//// shared/s.php
<?hh
// `unobservable` includes `shared`, so the strict-inclusion rule is satisfied;
// only the observed package's own flag rejects these.
function from_shared(): void {
  if (package unobservable) {
  }
}

<<__RequirePackage('unobservable')>>
function require_from_shared(): void {}

<<__SoftRequirePackage('unobservable')>>
function soft_require_from_shared(): void {}

//// intern/i.php
<?hh <<file: __PackageOverride('shared')>>
// The ban keys on the observed package, so relabelling the observer does not
// reach around it.
function from_overridden_observer(): void {
  if (package unobservable) {
  }
}
