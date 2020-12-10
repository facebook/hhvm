<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
async function f(): Awaitable<void> {
  await <<__NonRx(1)>>async {
    $a = 1;
  };
}
