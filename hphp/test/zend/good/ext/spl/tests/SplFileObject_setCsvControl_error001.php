<?php
file_put_contents('SplFileObject_setCsvControl_error001.csv',
<<<CDATA
'groene appelen'|10
'gele bananen'|20
'rode kersen'|30
CDATA
);
$s = new SplFileObject('SplFileObject_setCsvControl_error001.csv');
$s->setFlags(SplFileObject::READ_CSV);
$s->setCsvControl('||');
?>
<?php
unlink('SplFileObject_setCsvControl_error001.csv');
?>