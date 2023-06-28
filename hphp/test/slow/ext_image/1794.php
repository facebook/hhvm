<?hh


<<__EntryPoint>>
function main_1794() :mixed{

$exif = exif_read_data(__DIR__.'/images/246x247.png');
print_r($exif);


$exif = exif_read_data(__DIR__.'/images/php.gif');
print_r($exif);


$exif = exif_read_data(__DIR__.'/images/simpletext.jpg');
print_r($exif);


$exif = exif_read_data(__DIR__.'/images/smile.happy.png');
print_r($exif);


$exif = exif_read_data(__DIR__.'/images/test1pix.jpg');
print_r($exif);


$exif = exif_read_data(__DIR__.'/images/test2.jpg');
print_r($exif);
}
