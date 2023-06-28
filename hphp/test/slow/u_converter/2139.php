<?hh


<<__EntryPoint>>
function main_2139() :mixed{
var_dump(UConverter::getStandardName('latin1', 'MIME'));
var_dump(UConverter::getStandardName('latin1', 'IANA'));
var_dump(UConverter::getStandardName('blergh', 'blergh'));
}
