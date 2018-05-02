<?hh

function provide_constant_varray() {
  return varray[
    'first value',
    'second value',
  ];
}

// Provides the same as the above, but manually.
function provide_hhas_adata() {
  return __hhas_adata(
    "y:2:{s:11:\"first value\";s:12:\"second value\";}"
  );
}

var_export(provide_constant_varray() === provide_hhas_adata());
