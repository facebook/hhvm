<?hh
<<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
var_dump(count(transliterator_list_ids()) > 100);
var_dump(count(Transliterator::listIDs()) > 100);

echo "Done.\n";
}
