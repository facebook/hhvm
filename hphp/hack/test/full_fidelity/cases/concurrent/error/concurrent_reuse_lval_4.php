<?hh

async function f() {
  concurrent {
    await genx($x .= 'foo');
    await genx($x);
  }
}
