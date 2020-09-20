<?hh

<<__EntryPoint>>
function main_get_sort_key() {
$coll = new Collator('root');
var_dump(substr_count($coll->getSortKey('Hello'), "\0"));
}
