<?hh

// Test soft/nullable flag merging with EnforceOverriddenReturnTypes=1.
<<__EntryPoint>>
function main(): void {
  require_once(__DIR__.'/checksoftnullable.inc');
  mainImpl();
}
