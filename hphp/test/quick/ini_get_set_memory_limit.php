<?hh
<<__EntryPoint>> function main(): void {
var_dump( ini_get('memory_limit'));

var_dump(ini_set('memory_limit', '128M'));
var_dump( ini_get('memory_limit'));

var_dump(ini_set('memory_limit', '128G'));
var_dump( ini_get('memory_limit'));

var_dump(ini_set('memory_limit', '17000K'));
var_dump( ini_get('memory_limit'));

var_dump(ini_set('memory_limit', '136314880'));
var_dump( ini_get('memory_limit'));
}
