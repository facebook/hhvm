<?hh

function g(): int {
  return 1;
}


function f(): void {
  // OK: lambda is shallow
  $a = () ==> {
    g();
  };
}
