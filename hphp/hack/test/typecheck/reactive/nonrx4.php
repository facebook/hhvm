<?hh // partial

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
