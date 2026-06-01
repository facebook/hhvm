<?hh

// Test soft/nullable flag merging with EnforceOverriddenReturnTypes=2.
<<__EntryPoint>>
function main(): void {
  require_once(__DIR__.'/checksoftnullable.inc');
  mainImpl();
}
