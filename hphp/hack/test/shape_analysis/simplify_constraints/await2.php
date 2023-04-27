<?hh

async function f(Awaitable<dict<string, int>> $d): Awaitable<void> {
  $d = await $d;
  $d['k'] = 42;
  inspect($d);
}
