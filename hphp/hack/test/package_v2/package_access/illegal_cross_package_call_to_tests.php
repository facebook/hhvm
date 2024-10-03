//// __tests__/foo.php
<?hh
// package test
function test(): void {}

//// foo.php
<?hh
// package pkg1
// TODO: this should be an error as arbitrary packages should not
// be allowed to call into __tests__
function foo(): void { test (); }
