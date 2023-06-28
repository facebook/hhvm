<?hh

function zardoz() :mixed{
  invariant_violation('foobar');
}

function foobar() :mixed{
  zardoz();
}

<<__EntryPoint>>
function main() :mixed{
  foobar();
}
