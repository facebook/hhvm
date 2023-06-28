<?hh

function defaults() :mixed{ echo "in defaults\n"; }

<<__EntryPoint>>
function main()[rx_local] :mixed{
  $f = () ==> { defaults(); };
  $f();
}
