<?hh

async function f() {
  concurrent {
    await genx($x = 42);
    $x = await genx();
  }
}
