<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function genData(bool $b): Map<string, mixed> {
  $data = Map {};
  hh_show($data);

  // $data has type Map<var_1,var_2>

  if ($b) {
    $data['c'] = null;
    $data['b'] = 'foo';
    hh_show($data);
    // $data has type Map<string,?var_3>
    // If we UNIFY ?var_3 with mixed then we will get var_3:=mixed
    return $data;
  }
  hh_show($data);

  if ($b) {
    $data['a'] = 'hey';
  }
  hh_show($data);
  return $data;
}
