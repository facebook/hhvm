<?hh

<<__EntryPoint>>
function main_fgets_cr() :mixed{
    $fh = tmpfile();
    fwrite($fh, str_repeat('x', 8191) . "\r\rend");
    fseek($fh, 0);
    $i = 0;
    $f = fgets($fh);
    while($f) {
        echo $i,"\n"; $i++;
        $f = fgets($fh);
    }
}
