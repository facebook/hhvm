<?hh


<<__EntryPoint>>
function main_imageaffinematrixget() :mixed{
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              dict["x" => "a", "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              dict["x" => 0, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              dict["x" => -20, "y" => -20]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              dict["x" => 0x7fffff00, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              dict["x" => 0, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              dict["y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              dict["x" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              dict["x" => "a", "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              dict["x" => 0, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              dict["x" => -20, "y" => -20]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              dict["x" => 0x7fffff00, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              dict["x" => 0, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              dict["y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              dict["x" => 0]));

var_dump(imageaffinematrixget(IMG_AFFINE_ROTATE, 1));
var_dump(imageaffinematrixget(IMG_AFFINE_SHEAR_HORIZONTAL, 1));
var_dump(imageaffinematrixget(IMG_AFFINE_SHEAR_VERTICAL, 1));
}
