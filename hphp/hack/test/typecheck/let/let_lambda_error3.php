<?hh // experimental

function foo(): void {
  let x = 10;
  let f : (function(): string) = () ==> {
    return x;
  };
  f();
}
