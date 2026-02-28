<?hh


<<__EntryPoint>>
function main_immutable_createfromformat_invalid() :mixed{
var_dump(DateTimeImmutable::createFromFormat(DateTime::RFC3339, 'foo'));
}
