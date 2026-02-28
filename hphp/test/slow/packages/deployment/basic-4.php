<?hh

// this test ensures that directory internal_prod/ does NOT match
// the intern/ include_path: directory prod/internal_prod is in prod
// and the call must succeed, not in intern

<<__EntryPoint>>
function main_basic_4(): void {
  internal_prod_foo();
}
