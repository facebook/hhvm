<?hh


<<__EntryPoint>>
function main_hash_final_invalid() :mixed{
$ctx = hash_init('sha256' );
var_dump(hash_final($ctx));
var_dump(hash_final($ctx));
}
