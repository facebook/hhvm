<?hh // partial

function test(): void {
  goto LAMBDA;

  $x = $foo ==> {
    LAMBDA:
    return true;
  };
  return false;
}
