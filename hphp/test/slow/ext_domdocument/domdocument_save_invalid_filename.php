<?hh

<<__EntryPoint>>
function main_domdocument_save_invalid_filename() {
(new DOMDocument)->save("");
}
