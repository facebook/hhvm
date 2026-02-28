<?hh

const THE_CONST = 123;
function f($a=dict[THE_CONST=>THE_CONST]) :mixed{
  print_r($a);
}

<<__EntryPoint>> function main(): void {
echo "Begin\n";
f();
f();
f();
echo "Done";
}
