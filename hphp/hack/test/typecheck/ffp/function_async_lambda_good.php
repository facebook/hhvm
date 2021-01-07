<?hh

function foo(): void {
  $f = async () ==> 1;
  $g = async (): Awaitable<int> ==> 1;
}
