<?hh

function zardoz() {
  invariant_violation('foobar');
}

function foobar() {
  zardoz();
}

<<__EntryPoint>>
function main() {
  foobar();
}
