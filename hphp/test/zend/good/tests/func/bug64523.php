<?hh <<__EntryPoint>> function main(): void {
error_reporting(E_ALL ^ E_NOTICE ^ E_STRICT ^ E_DEPRECATED);
echo ini_get('error_reporting');
}
