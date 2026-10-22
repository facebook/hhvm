<?hh

function recursive_condition_impl(): bool {
  return false;
}

function recursive_condition(): bool {
  return recursive_condition_impl();
}

<<__AsioLowPri>>
async function recursive_low_pri(): Awaitable<void> {}

async function recursive_a(): Awaitable<void> {
  await recursive_b();
}

async function recursive_b(): Awaitable<void> {
  if (recursive_condition()) await recursive_low_pri();
  await recursive_a();
}

<<__EntryPoint>>
function main(): void {
  echo "DONE\n";
}
