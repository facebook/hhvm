<?hh

// Despite how improper the usage seems here,
// hhvm does not throw an error
// so we have decided that it is okay for this case to pass the typechecker
function foo(): void {
  null['a'] ?? 12;
}
