<?hh // strict

function foo(): void {
  $x = async coroutine () ==> {
    return 42;
  };
}
