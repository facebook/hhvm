//// __tests__.php
<?hh
// package test
function test(): void {}

//// foo.php
<?hh
// package pkg1
function foo(): void { test (); }
