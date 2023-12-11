<?hh

function test($v1, $v2) :mixed{
    $compare = version_compare($v1, $v2);
    switch ($compare) {
    case -1:
        print "$v1 < $v2\n";
        break;
    case 1:
        print "$v1 > $v2\n";
        break;
    case 0:
    default:
        print "$v1 = $v2\n";
        break;
    }
}
<<__EntryPoint>> function main(): void {
print "TESTING COMPARE\n";
$special_forms = vec["-dev", "a1", "b1", "RC1", "rc1", "", "pl1"];
$operators = vec[
    "lt", "<",
    "le", "<=",
    "gt", ">",
    "ge", ">=",
    "eq", "=", "==",
    "ne", "<>", "!="
];
test("1", "2");
test("10", "2");
test("1.0", "1.1");
test("1.2", "1.0.1");
foreach ($special_forms as $f1) {
    foreach ($special_forms as $f2) {
    test("1.0$f1", "1.0$f2");
    }
}
print "TESTING OPERATORS\n";
foreach ($special_forms as $f1) {
    foreach ($special_forms as $f2) {
        foreach ($operators as $op) {
            $v1 = "1.0$f1";
            $v2 = "1.0$f2";
            $test = version_compare($v1, $v2, $op) ? "true" : "false";
            printf("%7s %2s %-7s : %s\n", $v1, $op, $v2, $test);
        }
    }
}
}
