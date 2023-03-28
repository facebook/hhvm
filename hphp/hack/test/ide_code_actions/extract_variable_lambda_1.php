<?hh

<<__EntryPoint>>
function main(): void {
  // No variable extraction expected
  $z = 100 +
    () ==>/*range-start*/300 + 2/*range-end*/
}
