<?hh

<<__EntryPoint>>
function main_basic_1(): void {
  foo(); // in package prod

  quz(); // softly in prod
  // observe that qux() is softly included, but we do not warn about merge
  // because it is not called

  bar(); // in package intern
}
