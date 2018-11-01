<?hh

async function f() {
  concurrent {
    await genx(inout_fun(inout $x));
    await genx($x);
  }
}
