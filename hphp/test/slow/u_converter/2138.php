<?hh


<<__EntryPoint>>
function main_2138() :mixed{
$standards = UConverter::getStandards();
var_dump(in_array('IANA', $standards));
var_dump(in_array('MIME', $standards));
}
