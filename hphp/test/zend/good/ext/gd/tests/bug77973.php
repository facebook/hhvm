<?hh <<__EntryPoint>> function main(): void {
$im = imagecreatefromxbm(__DIR__ . '/bug77973.xbm');
var_dump($im);
}
