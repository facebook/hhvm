<?hh

function main() :mixed{
  // php://input is backed by a MemFile
  $f = fopen('php://input', 'r');
  var_dump(fstat($f));
}


<<__EntryPoint>>
function main_fstat_memfile() :mixed{
main();
}
