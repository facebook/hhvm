<?hh <<__EntryPoint>> function main(): void {
$img = imagecreatetruecolor(10, 10);

// POC #1
var_dump(imagecrop($img, dict["x" => "a", "y" => 0, "width" => 10, "height" => 10]));

$arr = dict["x" => "a", "y" => "12b", "width" => 10, "height" => 10];
var_dump(imagecrop($img, $arr));
print_r($arr);

// POC #2
var_dump(imagecrop($img, dict["x" => 0, "y" => 0, "width" => -1, "height" => 10]));

// POC #3
var_dump(imagecrop($img, dict["x" => -20, "y" => -20, "width" => 10, "height" => 10]));

// POC #4
var_dump(imagecrop($img, dict["x" => 0x7fffff00, "y" => 0, "width" => 10, "height" => 10]));

// bug 66815
var_dump(imagecrop($img, dict["x" => 0, "y" => 0, "width" => 65535, "height" => 65535]));
}
