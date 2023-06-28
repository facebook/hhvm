<?hh

<<__EntryPoint>>
function main_domdocument_save_invalid_filename() :mixed{
(new DOMDocument)->save("");
}
