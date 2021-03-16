<?hh

function defaults() { echo "in defaults\n"; }

<<__EntryPoint>>
function main()[rx_local] {
  $f = () ==> { defaults(); };
  $f();
}
