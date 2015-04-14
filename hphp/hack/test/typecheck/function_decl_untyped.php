//// decl.php
<?hh // decl

async function f_asyncgen() {
  yield 10;
}

function f_gen() {
  yield 10;
}

async function f_async() {
  return 10;
}

function f_sync() {
  return 10;
}

//// partial.php
<?hh

function test() {
  hh_show(f_asyncgen());
  hh_show(f_gen());
  hh_show(f_async());
  hh_show(f_sync());
}
