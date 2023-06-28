<?hh


<<__EntryPoint>>
function main_fpassthru() :mixed{
$f = fopen(__DIR__.'/test_ext_file.txt', 'r');
fpassthru($f);
}
