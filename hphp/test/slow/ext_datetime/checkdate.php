<?hh


<<__EntryPoint>>
function main_checkdate() {
var_dump(checkdate(12, 31, 2000));
var_dump(!checkdate(2, 29, 2001));
}
