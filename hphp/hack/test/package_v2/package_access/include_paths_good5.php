//// foo.php
<?hh
// package pkg1
function foo(): void {}

//// __tests__.php
<?hh
// package test
function test(): void { foo (); }
