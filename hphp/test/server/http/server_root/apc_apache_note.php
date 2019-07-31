<?hh

function apc_apache_note() {
  $ignore = false;
  apache_note('what', (string)apc_fetch('what', inout $ignore));
  apc_store('what', 'hello' . rand(100));
}

apc_apache_note();
var_dump('OK');
