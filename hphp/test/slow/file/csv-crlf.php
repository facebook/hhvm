<?hh

<<__EntryPoint>>
function main_csv_crlf() :mixed{
$fh = tmpfile();
fwrite($fh, str_repeat('x', 8191) . "\r\ny");
fseek($fh, 0);
$i = 0;
$f = fgetcsv($fh);
while($f) {
    echo $i,"\n"; $i++;
    $f = fgetcsv($fh);
}
}
