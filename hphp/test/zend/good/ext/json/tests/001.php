<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(json_decode()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(json_decode(""));
var_dump(json_decode("", true));
var_dump(json_decode("", false));
var_dump(json_decode(".", true));
var_dump(json_decode(".", false));
var_dump(json_decode("<?>"));
var_dump(json_decode(";"));
var_dump(json_decode("руссиш"));
var_dump(json_decode("blah"));
var_dump(json_decode('{ "test": { "foo": "bar" } }'));
var_dump(json_decode('{ "test": { "foo": "" } }'));
var_dump(json_decode('{ "": { "foo": "" } }'));
var_dump(json_decode('{ "": { "": "" } }'));
var_dump(json_decode('{ "": { "": "" }'));
var_dump(json_decode('{ "": "": "" } }'));

echo "===DONE===\n";
}
