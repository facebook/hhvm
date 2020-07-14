<?hh

function f(): dict<int,
  int>
{
  return "hello";
}

function g(): void {
  echo "hello";
}

<<__EntryPoint>>
function main(): void {
  g(1,
  2,
  3);
}
