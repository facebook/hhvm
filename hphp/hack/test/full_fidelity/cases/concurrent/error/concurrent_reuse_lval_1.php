<?hh

async function f() {
  concurrent {
    await genx($x = 42);
    await genx($x);
  }
}
