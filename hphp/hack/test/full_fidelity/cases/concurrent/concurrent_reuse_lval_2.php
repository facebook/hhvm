<?hh

async function f() {
  $x = 42;
  concurrent {
    $x = await genx();
    await genx($x);
  }
}
