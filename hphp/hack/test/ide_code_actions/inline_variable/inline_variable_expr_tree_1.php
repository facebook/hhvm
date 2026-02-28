<?hh

function foo(): void {
  ExampleDsl`() ==> {
    $x = 5;
   /*range-start*/$x/*range-end*/;
  }`;
}
