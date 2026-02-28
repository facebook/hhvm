<?hh

function _pow($e) :mixed{
  $a = 2 ** $e;
  $e **= 2;
}

<<__EntryPoint>>
function main_pow_simple() :mixed{
echo "ok";
}
