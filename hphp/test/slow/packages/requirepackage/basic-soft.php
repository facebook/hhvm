<?hh
// package foo

// ERROR - package softbar is soft-deployed in the same deployment as package foo
<<__RequirePackage("softbar")>>
function a(): void {
  var_dump("in a");
}

<<__EntryPoint>>
function basic_soft(): void {
  a();
}
