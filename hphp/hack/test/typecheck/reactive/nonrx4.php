<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// OK
async function f(): Awaitable<void> {
  $a = <<__NonRx>>() ==> 1;
  $b = <<__NonRx>>function () {
    return 1;
  };
  await <<__NonRx>>async {
    $a = 1;
  };
}
