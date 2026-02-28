<?hh

function apc_apache_note() :mixed{
  $ignore = false;
  apache_note('what', (string)apc_fetch('what', inout $ignore));
  apc_store('what', 'hello' . rand(100));
}

<<__EntryPoint>>
function apc_apache_note_entrypoint() :mixed{
apc_apache_note();
var_dump('OK');
}
