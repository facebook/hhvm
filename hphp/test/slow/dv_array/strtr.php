<?hh
<<__EntryPoint>> function main(): void {
var_dump(strtr("-42", dict["--" => "-", "-" => "."]));
}
