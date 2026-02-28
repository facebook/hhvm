<?hh

function main() :mixed{
  $root = new DOMElement('html');
  $copy = clone $root;
}


<<__EntryPoint>>
function main_clone_crash() :mixed{
main();
echo "Done.\n";
}
