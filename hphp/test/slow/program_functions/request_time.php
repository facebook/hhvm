<?hh

function dump(string $name) {
  $val = $_SERVER[$name];
  $len = HH\Lib\Str\length(HH\Lib\Str\split((string)$val, '.')[0]);
  echo "$name: $val $len\n";
}

<<__EntryPoint>>
function main() {
  dump('REQUEST_TIME');
  dump('REQUEST_TIME_FLOAT');
  dump('REQUEST_TIME_NS');
}
