<?hh

function test(): void {
  $x = 3;
  function() use ($x) {
    await null;
  };
}

<<__EntryPoint>> function main(): void {}
