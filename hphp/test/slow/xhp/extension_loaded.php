<?hh



<<__EntryPoint>>
function main_extension_loaded() {
var_dump(extension_loaded('xhp'));
var_dump(in_array('xhp', get_loaded_extensions()));
}
