<?hh


function array_extend(inout $dest, varray $src): void {
  foreach ($src as $element) {
    $dest[] = $element;
  }
}

function getImpliedChecks(varray $arg): varray {
  $implied_checks = vec[];
  foreach ($arg as $key => $val) {
    array_extend(inout $implied_checks, $val);
  }

  return $implied_checks;
}


<<__EntryPoint>>
function main_bug_3715777() :mixed{
for ($i = 0; $i < 10; $i++) {
  var_dump(getImpliedChecks(vec[vec[1,2,3], vec[4,5,6], vec[7,8,9]]));
}
}
