<?hh // partial

<<__Rx>>
async function f(): Awaitable<void> {
  await <<__NonRx(1)>>async {
    $a = 1;
  };
}
