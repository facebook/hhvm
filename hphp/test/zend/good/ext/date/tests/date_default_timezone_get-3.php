<?hh <<__EntryPoint>> function main(): void {
echo date_default_timezone_get(), "\n";

date_default_timezone_set("America/Chicago");
echo date_default_timezone_get(), "\n";
}
