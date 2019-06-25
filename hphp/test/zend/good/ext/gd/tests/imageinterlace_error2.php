<?hh <<__EntryPoint>> function main(): void {
$image = fopen('php://stdin', 'r');
var_dump(imageinterlace($image));
}
