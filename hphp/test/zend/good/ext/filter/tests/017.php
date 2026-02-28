<?hh
<<__EntryPoint>> function main(): void {
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, dict["options"=>dict["regexp"=>'/.*/']]));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, dict["options"=>dict["regexp"=>'/^b(.*)/']]));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, dict["options"=>dict["regexp"=>'/^d(.*)/']]));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, dict["options"=>dict["regexp"=>'/blah/']]));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, dict["options"=>dict["regexp"=>'/\[/']]));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP));

echo "Done\n";
}
