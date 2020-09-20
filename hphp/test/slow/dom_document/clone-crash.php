<?hh

function main() {
  $root = new DOMElement('html');
  $copy = clone $root;
}


<<__EntryPoint>>
function main_clone_crash() {
main();
echo "Done.\n";
}
