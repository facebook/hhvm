<?hh

function foo(): void {
  // Offering "inline variable" refactoring is incorrect here,
  // since nested expression trees are not *yet* supported.
  // But nesting may be supported soon, so leaving behavior as-is.
  $x = ExampleDsl`() ==> {}`;
  ExampleDsl`() ==> {
   /*range-start*/$x/*range-end*/;
  }`;
}
