<?hh

async function f() {
  concurrent {
    await genx($x->y = 42);
    await genx();
  }
}
