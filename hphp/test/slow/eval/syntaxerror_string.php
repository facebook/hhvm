<?hh

<<__EntryPoint>>
function main_syntaxerror_string() :mixed{
print "Before error\n";
$result = eval("function foo() { echo foo }");
if ($result === false) {
  echo "eval returns false\n";
} else {
  echo "eval does not return false\n";
}
}
