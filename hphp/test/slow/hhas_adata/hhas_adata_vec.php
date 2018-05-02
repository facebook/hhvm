<?hh

function provide_constant_vec() {
  return vec[
    'first value',
    'second value',
  ];
}

// Provides the same as the above, but manually.
function provide_hhas_adata() {
  return __hhas_adata(
    "v:2:{s:11:\"first value\";s:12:\"second value\";}"
  );
}

var_export(provide_constant_vec() === provide_hhas_adata());
