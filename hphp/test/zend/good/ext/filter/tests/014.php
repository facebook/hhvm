<?hh

class test {

    function __toString() :mixed{
        return "blah";
    }
}
<<__EntryPoint>> function main(): void {
$t = new test;

var_dump(filter_var("no", FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var(NULL, FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var($t, FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var(vec[1,2,3,0,vec["", "123"]], FILTER_VALIDATE_BOOLEAN, FILTER_REQUIRE_ARRAY));
var_dump(filter_var("yes", FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var("true", FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var("false", FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var("off", FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var("on", FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var("0", FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var("1", FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var("NONE", FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var("", FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var(-1, FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var("000000", FILTER_VALIDATE_BOOLEAN));
var_dump(filter_var("111111", FILTER_VALIDATE_BOOLEAN));


echo "Done\n";
}
