<?hh


<<__EntryPoint>>
function main_hash_equals_error() {
var_dump(hash_equals('', null));
var_dump(hash_equals(null, ''));
}
