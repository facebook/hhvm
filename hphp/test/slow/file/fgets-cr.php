<?hh

<<__EntryPoint>>
function main_fgets_cr() :mixed{
    $fh = tmpfile();
    fwrite($fh, str_repeat('x', 8191) . "\r\rend");
    fseek($fh, 0);
    $i = 0;
    while($f = fgets($fh)) {
        echo $i++,"\n";
    }
}
