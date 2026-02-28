<?hh


<<__EntryPoint>>
function main_null_index() :mixed{
echo "=== indexing into a boolean ===\n";
$t = false[2]; //  QueryM 1 CGet EI:2, return ElemEmptyish

echo "=== indexing into invalid keys ===\n";
$arr = darray(dict['key' => 'val']);
try { $t = $arr['invalid key']['invalid key 2']; } catch (Exception $e) { echo $e->getMessage()."\n"; }
$t = $arr['key']['invalid key 2'];
}
