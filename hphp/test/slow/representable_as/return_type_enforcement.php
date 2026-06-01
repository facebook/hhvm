<?hh

// RepresentableAs<int> should enforce 'int' at runtime
function returns_bad(): HH\Runtime\RepresentableAs<int> {
  return HH\FIXME\UNSAFE_CAST<string, HH\Runtime\RepresentableAs<int>>("hello");
}

<<__EntryPoint>>
function main(): void {
  returns_bad();
}
