<?php
file_put_contents('SplFileObject_setCsvControl_basic.csv',
<<<CDATA
'groene appelen'|10
'gele bananen'|20
'rode kersen'|30
CDATA
);
$s = new SplFileObject('SplFileObject_setCsvControl_basic.csv');
$s->setFlags(SplFileObject::READ_CSV);
$s->setCsvControl('|', '\'', '/');
foreach ($s as $row) {
    list($fruit, $quantity) = $row;
    echo "$fruit : $quantity\n";
}
?>
<?php
unlink('SplFileObject_setCsvControl_basic.csv');
?>