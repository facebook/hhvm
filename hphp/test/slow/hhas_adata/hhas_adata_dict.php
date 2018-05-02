<?hh

function provide_constant_dict() {
  return dict[
    'first key' => 'first value',
    'second key' => 'second value',
  ];
}

// Provides the same as the above, but manually.
function provide_hhas_adata() {
  return __hhas_adata(
    "D:2:{s:9:\"first key\";s:11:\"first value\";s:10:\"second key\";s:12:\"second value\";}"
  );
}

var_export(provide_constant_dict() === provide_hhas_adata());
