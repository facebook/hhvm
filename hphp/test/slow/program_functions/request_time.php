<?hh

<<__EntryPoint>>
function main() {
  $val = $_SERVER['REQUEST_TIME_NS'];
  $len = HH\Lib\Str\length(HH\Lib\Str\split((string)$val, '.')[0]);
  echo "REQUEST_TIME_NS: $val $len\n";
}
