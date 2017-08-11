<?hh
//AUTOCOMPLETE 9 31
function test_local_var_completion(int $function_param) : bool {
  return true;
}

function run(){
  $local_variable = 5;
  test_local_var_completion($l);
}
