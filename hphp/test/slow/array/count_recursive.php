<?hh

function run(&$ref, $arr) {
  $ref = $arr;
  var_dump(count($arr, COUNT_RECURSIVE));
}

<<__EntryPoint>>
function main() {
  $arr = [];
  run(&$arr[], $arr);
}
