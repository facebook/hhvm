<?hh

<<__EntryPoint>>
function main_return_true() {
$result = eval("return true;");
if ($result === true) {
  echo "eval returns true\n";
} else {
  echo "eval does not return true\n";
}
}
