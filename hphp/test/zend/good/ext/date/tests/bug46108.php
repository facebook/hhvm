<?hh
<<__EntryPoint>> function main(): void {
date_default_timezone_set('America/Sao_Paulo');

var_dump(unserialize(serialize(new DateTime)));
}
