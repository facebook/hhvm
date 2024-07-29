<?hh

<<__EntryPoint>>
function main() {
  $val = \HH\global_get('_SERVER')['REQUEST_TIME_NS'];
  $len = HH\Lib\Str\length(HH\Lib\Str\split((string)$val, '.')[0]);
  echo "REQUEST_TIME_NS: $val $len\n";
}
