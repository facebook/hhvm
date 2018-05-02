<?hh

function provide_constant_darray() {
  return darray[
    'first key' => 'first value',
    'second key' => 'second value',
  ];
}

// Provides the same as the above, but manually.
function provide_hhas_adata() {
  return __hhas_adata(
    "Y:2:{s:9:\"first key\";s:11:\"first value\";s:10:\"second key\";s:12:\"second value\";}"
  );
}

var_export(provide_constant_darray() === provide_hhas_adata());
