<?hh <<__EntryPoint>> function main(): void {
$output = null;
$return_var = -1;
exec("echo x", inout $output, inout $return_var);
var_dump($output);
}
