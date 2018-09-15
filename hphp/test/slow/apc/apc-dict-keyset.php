<?hh

<<__EntryPoint>>
function main_apc_dict_keyset() {
apc_store("apc-dict-keyset", dict['k' => keyset[rand()]], 1);
apc_delete("apc-dict-keyset");
}
