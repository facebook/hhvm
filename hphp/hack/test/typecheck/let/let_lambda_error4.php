<?hh // experimental

function foo(): void {
  let x = 10;
  let f : (function(string): string) = ($s) ==> {
    return $s."string";
  };
  f(x);
}
