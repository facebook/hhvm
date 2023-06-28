<?hh


function array_extend(inout $dest, varray $src): void {
  foreach ($src as $element) {
    $dest[] = $element;
  }
}

function getImpliedChecks(varray $arg): varray {
  $implied_checks = varray[];
  foreach ($arg as $key => $val) {
    array_extend(inout $implied_checks, $val);
  }

  return $implied_checks;
}


<<__EntryPoint>>
function main_bug_3715777() :mixed{
for ($i = 0; $i < 10; $i++) {
  var_dump(getImpliedChecks(varray[varray[1,2,3], varray[4,5,6], varray[7,8,9]]));
}
}
