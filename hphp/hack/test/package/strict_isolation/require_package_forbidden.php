//// intern/hard.php
<?hh
// __RequirePackage referencing a strict-isolation package is an error.
<<__RequirePackage('isolated')>>
function test_require(): void {}

//// intern/soft.php
<?hh
// __SoftRequirePackage is likewise forbidden for a strict-isolation package.
<<__SoftRequirePackage('isolated')>>
function test_soft_require(): void {}

//// intern/soft_sampled.php
<?hh
// The optional integer sampling rate must not let __SoftRequirePackage bypass
// the strict-isolation check.
<<__SoftRequirePackage('isolated', 100)>>
function test_soft_require_sampled(): void {}
