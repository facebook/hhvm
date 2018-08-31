<?hh

function _pow($e) {
  $a = 2 ** $e;
  $e **= 2;
}

<<__EntryPoint>>
function main_pow_simple() {
echo "ok";
}
