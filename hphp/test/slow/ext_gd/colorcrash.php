<?hh

<<__EntryPoint>>
function main_colorcrash() :mixed{
$img=imagecreatetruecolor(10, 10);
imagetruecolortopalette($img, false, (int)(PHP_INT_MAX / 8));

$img = imagecreate(100, 100);
imageline($img, 0, 0, 100, 100, -7);
imagepolygon($img, vec[10,10,10,50,50,50,50,10,10,10], 5, -7);
echo "done\n";
}
