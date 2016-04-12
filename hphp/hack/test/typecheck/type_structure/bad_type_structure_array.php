<?hh // strict

function concrete(TypeStructure<array<int>> $ts): void {
  hh_show($ts['kind']);
  hh_show($ts['alias']);
  hh_show($ts['name']);
  hh_show($ts['generic_types']);

  // any other field will fail
  hh_show($ts['classname']);
  hh_show($ts['nullable']);
  hh_show($ts['elem_types']);
  hh_show($ts['fields']);
  hh_show($ts['param_types']);
  hh_show($ts['return_type']);

  // make sure kind still works
  hh_show($ts['kind']);
}

function generic<T as array<int>>(TypeStructure<T> $ts): void {
  hh_show($ts['kind']);
  hh_show($ts['alias']);
  hh_show($ts['name']);
  hh_show($ts['generic_types']);

  // any other field will fail
  hh_show($ts['classname']);
  hh_show($ts['nullable']);
  hh_show($ts['elem_types']);
  hh_show($ts['fields']);
  hh_show($ts['param_types']);
  hh_show($ts['return_type']);

  // make sure kind still works
  hh_show($ts['kind']);
}
