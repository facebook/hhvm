<?hh

function defaults()[defaults]: void {
  echo "in defaults\n";
}

async function defaults_async()[defaults]: Awaitable<void> {
  echo "in defaults_async\n";
}

<<__EntryPoint>>
async function main()[] {
  defaults();
  HH\Coeffects\backdoor(defaults<>);
  await defaults_async();
  HH\Coeffects\backdoor(defaults_async<>);

  HH\Coeffects\backdoor(()[policied_of] ==> { echo "in policied_of\n"; });
}
