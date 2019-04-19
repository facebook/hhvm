<?hh

function foo(bar\int $a): int {
    return $a;
}

<<__EntryPoint>>
function main(): void {
  foo(10);
}
