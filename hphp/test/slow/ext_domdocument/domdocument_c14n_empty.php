<?hh

<<__EntryPoint>>
function main_domdocument_c14n_empty() {
$doc = new DOMDocument();
var_dump($doc->C14N());
}
