<?hh
<<__EntryPoint>> function main(): void {
$im = new Imagick();
$im->newImage(1000,1000, "white","png");

$draw = new ImagickDraw();
$draw->setFont(__DIR__.'/anonymous_pro_minus.ttf');
$draw->setFontSize(72.0);

$draw->setResolution(10.0, 10.0);
$small = $im->queryFontMetrics($draw, "Hello World");

$draw->setResolution(300.0, 300.0);
$large = $im->queryFontMetrics($draw, "Hello World");

var_dump($small['textWidth'] < $large['textWidth']);

echo "OK\n";
}
