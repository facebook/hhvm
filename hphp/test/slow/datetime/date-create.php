<?hh


<<__EntryPoint>>
function main_date_create() :mixed{
var_dump(date_create('foo'));
var_dump(date_create_immutable('foo'));
}
