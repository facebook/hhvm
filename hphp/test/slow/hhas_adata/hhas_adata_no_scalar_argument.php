<?hh


<<__EntryPoint>>
function main_hhas_adata_no_scalar_argument() {
try {
  __hhas_adata('arg1', 'arg2');
  print "FAILED\n";
} catch (Exception $e) {
  print $e->getMessage();
}
}
