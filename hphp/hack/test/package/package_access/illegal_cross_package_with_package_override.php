//// pkg6/foo.php
<?hh
// foo overrides down into pkg1 (valid: pkg6 includes pkg1)
<<file: __PackageOverride('pkg1')>>
function foo(): void {}

//// pkg4/bar.php
<?hh
// pkg4 is unrelated to pkg1 and so cannot reach foo
function bar(): void { foo (); }
