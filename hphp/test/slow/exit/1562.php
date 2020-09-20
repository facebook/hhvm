<?hh

function callback($data) {
  return "callback: $data";
}

<<__EntryPoint>>
function main_1562() {
ob_start();
echo "from first level\n";
ob_start();
ob_start(fun("callback"));
echo "foobar!\n";
exit;
}
