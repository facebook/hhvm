<?hh
<<__EntryPoint>> function main(): void {
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, darray["options"=>darray["regexp"=>'/.*/']]));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, darray["options"=>darray["regexp"=>'/^b(.*)/']]));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, darray["options"=>darray["regexp"=>'/^d(.*)/']]));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, darray["options"=>darray["regexp"=>'/blah/']]));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP, darray["options"=>darray["regexp"=>'/\[/']]));
var_dump(filter_var("data", FILTER_VALIDATE_REGEXP));

echo "Done\n";
}
