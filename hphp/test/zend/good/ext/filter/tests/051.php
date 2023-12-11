<?hh <<__EntryPoint>> function main(): void {
$tmp = $default = 321;
var_dump(filter_var("123asd", FILTER_VALIDATE_INT, dict["options"=>dict["default"=>$default]]));
}
