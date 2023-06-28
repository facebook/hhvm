<?hh

function defaults()[defaults]: void {
  echo "in defaults\n";
}

async function defaults_async()[defaults]: Awaitable<void> {
  echo "in defaults_async\n";
}

<<__EntryPoint>>
async function main()[] :Awaitable<mixed>{
  defaults();
  HH\Coeffects\backdoor(defaults<>);
  await defaults_async();
  HH\Coeffects\backdoor(defaults_async<>);

  HH\Coeffects\backdoor(()[zoned_with] ==> { echo "in zoned_with\n"; });
}
