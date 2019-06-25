<?hh
<<__EntryPoint>> function main(): void {
$email  = 'AbcCdEfGh';
var_dump(stristr($email, 'c'));
var_dump(stristr($email, 'c', true));

$email  = 'AbCdeEfGh';
var_dump(stristr($email, 'E'));
var_dump(stristr($email, 'E', true));

$email  = 'wazAbCdeEfGh';
var_dump(stristr($email, 97));
var_dump(stristr($email, 97, true));
}
