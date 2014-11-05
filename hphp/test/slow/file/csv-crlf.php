<?php
$fh = tmpfile();
fwrite($fh, str_repeat('x', 8191) . "\r\ny");
fseek($fh, 0);
$i = 0;
while($f = fgetcsv($fh)) {
    echo $i++,"\n";
}
