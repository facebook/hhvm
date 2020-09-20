<?hh

async function f() {
  concurrent $x = await genx();
}
