<?hh

function provide_constant_darray() {
  return darray[
    'first key' => 'first value',
    'second key' => 'second value',
  ];
}

// Provides the same as the above, but using __hhas_adata with a nowdoc.
function provide_hhas_adata_nowdoc() {
  return __hhas_adata(<<<'NOWDOC'
Y:2:{s:9:"first key";s:11:"first value";s:10:"second key";s:12:"second value";}
NOWDOC
  );
}

// Provides the same as the above, but using __hhas_adata with a single-quoted
// string.
function provide_hhas_adata_single_quoted() {
  return __hhas_adata(
    'Y:2:{s:9:"first key";s:11:"first value";s:10:"second key";s:12:"second value";}'
  );
}

// Provides the same as the above, but using __hhas_adata with a double-quoted
// string.
function provide_hhas_adata_double_quoted() {
  return __hhas_adata(
    "Y:2:{s:9:\"first key\";s:11:\"first value\";s:10:\"second key\";s:12:\"second value\";}"
  );
}


<<__EntryPoint>>
function main_hhas_adata_darray() {
var_dump(provide_constant_darray() === provide_hhas_adata_nowdoc());
var_dump(provide_constant_darray() === provide_hhas_adata_single_quoted());
var_dump(provide_constant_darray() === provide_hhas_adata_double_quoted());
}
