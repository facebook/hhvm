<?hh


<<__EntryPoint>>
function pseudomains_suck() {
  var_dump(__FUNCTION__, get_defined_constants(true)['user']);
}

const FOO = 42;
const BAR = FOO + 1;
const SOME_CONSTANT = "some string";
$a0 = get_defined_constants(true)['user'];
const ANOTHER_CONSTANT = "some other string";
$a1 = get_defined_constants(true)['user'];
const FOO = BAR;
const BAR = BAR;
$a2 = get_defined_constants(true)['user'];
var_dump(array_diff($a1, $a0));
var_dump(array_diff($a2, $a0) === array_diff($a1, $a0));
