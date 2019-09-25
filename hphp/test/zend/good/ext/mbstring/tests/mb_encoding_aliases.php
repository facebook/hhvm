<?hh <<__EntryPoint>> function main(): void {
try { mb_encoding_aliases(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$list = mb_encoding_aliases("ASCII");
sort(inout $list);
var_dump($list);
var_dump(mb_encoding_aliases("7bit"));
var_dump(mb_encoding_aliases("8bit"));
}
