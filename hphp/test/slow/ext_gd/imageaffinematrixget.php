<?hh


<<__EntryPoint>>
function main_imageaffinematrixget() :mixed{
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              darray["x" => "a", "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              darray["x" => 0, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              darray["x" => -20, "y" => -20]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              darray["x" => 0x7fffff00, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              darray["x" => 0, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              darray["y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              darray["x" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              darray["x" => "a", "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              darray["x" => 0, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              darray["x" => -20, "y" => -20]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              darray["x" => 0x7fffff00, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              darray["x" => 0, "y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              darray["y" => 0]));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              darray["x" => 0]));

var_dump(imageaffinematrixget(IMG_AFFINE_ROTATE, 1));
var_dump(imageaffinematrixget(IMG_AFFINE_SHEAR_HORIZONTAL, 1));
var_dump(imageaffinematrixget(IMG_AFFINE_SHEAR_VERTICAL, 1));
}
