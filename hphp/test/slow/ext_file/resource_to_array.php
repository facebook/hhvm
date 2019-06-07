<?hh

function main() {
  $tempfile = tempnam('/tmp', 'vmextfiletest');
  $f = fopen($tempfile, 'w');
  var_dump((array)$f);
  unlink($tempfile);
}


<<__EntryPoint>>
function main_resource_to_array() {
main();
}
