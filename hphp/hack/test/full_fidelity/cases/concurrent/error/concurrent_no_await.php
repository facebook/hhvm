<?hh

async function f() {
  concurrent {
    await genx();
    geny();
  }
}
