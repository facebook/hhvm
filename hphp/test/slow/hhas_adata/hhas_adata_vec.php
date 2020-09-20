<?hh

function provide_constant_vec() {
  return vec[
    'first value',
    'second value',
  ];
}

// Provides the same as the above, but using __hhas_adata with a nowdoc.
function provide_hhas_adata_nowdoc() {
  return __hhas_adata(<<<'NOWDOC'
v:2:{s:11:"first value";s:12:"second value";}
NOWDOC
  );
}

// Provides the same as the above, but using __hhas_adata with a single-quoted
// string.
function provide_hhas_adata_single_quoted() {
  return __hhas_adata(
    'v:2:{s:11:"first value";s:12:"second value";}'
  );
}

// Provides the same as the above, but using __hhas_adata with a double-quoted
// string.
function provide_hhas_adata_double_quoted() {
  return __hhas_adata(
    "v:2:{s:11:\"first value\";s:12:\"second value\";}"
  );
}


<<__EntryPoint>>
function main_hhas_adata_vec() {
var_dump(provide_constant_vec() === provide_hhas_adata_nowdoc());
var_dump(provide_constant_vec() === provide_hhas_adata_single_quoted());
var_dump(provide_constant_vec() === provide_hhas_adata_double_quoted());
}
