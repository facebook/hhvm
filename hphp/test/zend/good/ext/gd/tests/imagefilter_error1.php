<?hh <<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(180, 30);

try { var_dump(imagefilter($image)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
