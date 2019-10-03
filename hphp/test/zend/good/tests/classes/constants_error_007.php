<?hh

function by_ref(inout $ref) {}

class aclass {
  const myConst = "hello";
}
<<__EntryPoint>> function main(): void {
echo "\nAttempting to create a reference to a class constant - should be parse error.\n";
by_ref(inout aclass::myConst);
}
