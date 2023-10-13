<?hh

async function f(): Awaitable<string> {
  /* HH_FIXME[1002]: check parser error recovery */
  $x = () ==> async {
    return 10;
  };
  return await $x();
}
