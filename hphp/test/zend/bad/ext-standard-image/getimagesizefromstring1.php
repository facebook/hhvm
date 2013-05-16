<?PHP
$img = __DIR__ . '/test.gif';

$i1 = getimagesize($img);
    
$data = file_get_contents($img);
    
$i2 = getimagesizefromstring($data);

var_dump($i1);
var_dump($i2);
?>