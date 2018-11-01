<?hh

async function f() {
  concurrent {
    await genx(ref_fun(&$x));
    await genx($x);
  }
}
