<?hh
<<__EntryPoint>> function main(): void {
date_default_timezone_set('America/Los_Angeles');
var_dump(DateTime::createFromFormat('Y/m/d H:i:s', '1995/06/08 12:34:56', null));
var_dump(DateTimeImmutable::createFromFormat('Y/m/d H:i:s', '1995/06/08 12:34:56', null));
}
