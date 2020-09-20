<?hh <<__EntryPoint>> function main(): void {
var_dump(filter_var("FC00::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var("FC00::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6 | FILTER_FLAG_NO_PRIV_RANGE));
var_dump(filter_var("::", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var("::", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6 | FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var("::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6 | FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("fe8:5:6::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var("fe8:5:6::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6 | FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("2001:0db8::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var("2001:0db8::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6 | FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("5f::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var("5f::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6 | FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var("3ff3::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var("3ff3::1", FILTER_VALIDATE_IP, FILTER_FLAG_IPV6 | FILTER_FLAG_NO_RES_RANGE));
}
