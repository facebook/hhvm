<?hh <<__EntryPoint>> function main(): void {
var_dump (0.002877 == 2877.0 / 1000000.0);
var_dump (substr (sprintf ("%.35f", 0.002877), 0, 10));
}
