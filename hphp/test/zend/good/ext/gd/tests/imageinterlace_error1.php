<?hh
<<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(100, 100);

//calling with no parameters
try { var_dump(imageinterlace()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
