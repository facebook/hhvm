<?hh

<<__NEVER_INLINE>>
function foo(bool $throw): void {
  if ($throw) {
    throw new Exception();
  }
}

async function bar(bool $throw): Awaitable<void> {
  foo($throw);
}


<<__EntryPoint>>
function main(): void {
  foreach (vec[false, true] as $throw) {
    bar($throw);
  }
  echo "done\n";
}
