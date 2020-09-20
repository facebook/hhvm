<?hh
<<__EntryPoint>> function main(): void {
error_reporting(0);

$img = new Imagick();
echo $img->foobar;

echo "OK";
}
