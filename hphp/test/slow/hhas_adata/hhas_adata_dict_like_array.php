<?hh

function provide_constant_dict_like_array() {
  return darray[
    'first key' => 'first value',
    'second key' => 'second value',
  ];
}

// Provides the same as the above, but using __hhas_adata with a nowdoc.
function provide_hhas_adata_nowdoc() {
  return __hhas_adata(<<<'NOWDOC'
a:2:{s:9:"first key";s:11:"first value";s:10:"second key";s:12:"second value";}
NOWDOC
  );
}

// Provides the same as the above, but using __hhas_adata with a single-quoted
// string.
function provide_hhas_adata_single_quoted() {
  return __hhas_adata(
    'a:2:{s:9:"first key";s:11:"first value";s:10:"second key";s:12:"second value";}'
  );
}

// Provides the same as the above, but using __hhas_adata with a double-quoted
// string.
function provide_hhas_adata_double_quoted() {
  return __hhas_adata(
    "a:2:{s:9:\"first key\";s:11:\"first value\";s:10:\"second key\";s:12:\"second value\";}"
  );
}


<<__EntryPoint>>
function main_hhas_adata_dict_like_array() {
var_dump(provide_constant_dict_like_array() === provide_hhas_adata_nowdoc());
var_dump(
  provide_constant_dict_like_array() === provide_hhas_adata_single_quoted(),
);
var_dump(
  provide_constant_dict_like_array() === provide_hhas_adata_double_quoted(),
);
}
