<?hh

async function f(): Awaitable<string> {
  $x = () ==> async {
    return 'foo';
  };
  return await $x();
}
