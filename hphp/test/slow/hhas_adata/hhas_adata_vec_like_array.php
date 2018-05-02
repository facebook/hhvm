<?hh

function provide_constant_vec_like_array() {
  return array(
    'first value',
    'second value',
  );
}

// Provides the same as the above, but manually.
function provide_hhas_adata() {
  return __hhas_adata(
    "a:2:{i:0;s:11:\"first value\";i:1;s:12:\"second value\";}"
  );
}

var_export(provide_constant_vec_like_array() === provide_hhas_adata());
