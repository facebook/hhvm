<?hh

function test(dict<string, int> $d): void {
  $key = "foo";
  $d[$key];
//^ enforcement-at-caret
}
