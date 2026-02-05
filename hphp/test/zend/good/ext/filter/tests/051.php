<?hh <<__EntryPoint>> function main(): void {
$default = 321;
$tmp = $default;
var_dump(filter_var("123asd", FILTER_VALIDATE_INT, dict["options"=>dict["default"=>$default]]));
}
