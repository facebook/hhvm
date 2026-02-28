<?hh

function test($tr_data) :mixed{
  $temp_tr = null;
  foreach ($tr_data as $tr_id => $tr_row) {
    if ($tr_row == 45) $temp_tr = $tr_id;
    if ((int)$tr_id == 0) {
      continue;
    }
 else {
      return $tr_row;
    }
  }
  if ($temp_tr) {
    return $temp_tr;
  }
  return null;
}

<<__EntryPoint>>
function main_1845() :mixed{
var_dump(test(dict['a' => 1, 'b' => 45]));
}
