<?hh

<<__EntryPoint>>
function main() {
  apc_store('mainf', fun('main'));
  apc_store('maina', array(1, fun('main'), 'foo'));

  var_dump(apc_fetch('mainf'));
  var_dump(apc_fetch('maina'));
}
