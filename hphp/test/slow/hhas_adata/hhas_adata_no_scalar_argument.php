<?hh

try {
  __hhas_adata('arg1', 'arg2');
  print "FAILED\n";
} catch (Exception $e) {
  print $e->getMessage();
}
