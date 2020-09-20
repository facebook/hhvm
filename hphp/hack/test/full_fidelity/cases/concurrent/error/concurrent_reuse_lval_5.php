<?hh

async function f() {
  concurrent {
    await genx($x++);
    await genx($x);
  }
}
