<?hh

const THE_CONST = 123;
function f($a=array(THE_CONST=>THE_CONST)) {
  print_r($a);
}

<<__EntryPoint>> function main() {
echo "Begin\n";
f();
f();
f();
echo "Done";
}
