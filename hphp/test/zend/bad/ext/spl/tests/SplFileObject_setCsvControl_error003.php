<?php
file_put_contents('csv_control_data.csv',
<<<CDATA
'groene appelen'|10
'gele bananen'|20
'rode kersen'|30
CDATA
);
$s = new SplFileObject('csv_control_data.csv');
$s->setFlags(SplFileObject::READ_CSV);
$s->setCsvControl('|', '\'', 'three');
?>
<?php
unlink('csv_control_data.csv');
?>