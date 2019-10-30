<?hh
<<__EntryPoint>> function main(): void {
try {
} finally {
  try {
     goto foo;
  }
  finally {}
}
foo:
}
