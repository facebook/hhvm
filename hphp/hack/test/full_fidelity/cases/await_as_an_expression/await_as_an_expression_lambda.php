<?hh

async function foo(): Awaitable<void> {
  $x = async { await genx(); };
  $x = async { f(await genx()); };
  $x = async () ==> await genx();
  $x = async () ==> f(await genx());
  $x = async () ==> { await genx(); };
  $x = async () ==> { f(await genx()); };
  $x = async function () { await genx(); };
  $x = async function () { f(await genx()); };
}
