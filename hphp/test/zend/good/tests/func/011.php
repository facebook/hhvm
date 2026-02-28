<?hh <<__EntryPoint>> function main(): void {
error_reporting(E_ALL & E_NOTICE | E_STRICT ^ E_DEPRECATED & ~E_WARNING);
echo ini_get('error_reporting');
}
