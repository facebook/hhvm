<?hh

async function f() {
  concurrent {
    $x = await genx($x = 42);
    await genx();
  }
}
