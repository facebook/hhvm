<?php

var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              array("x" => "a", "y" => 0)));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              array("x" => 0, "y" => 0)));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              array("x" => -20, "y" => -20)));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              array("x" => 0x7fffff00, "y" => 0)));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              array("x" => 0, "y" => 0)));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              array("y" => 0)));
var_dump(imageaffinematrixget(IMG_AFFINE_TRANSLATE,
                              array("x" => 0)));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              array("x" => "a", "y" => 0)));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              array("x" => 0, "y" => 0)));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              array("x" => -20, "y" => -20)));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              array("x" => 0x7fffff00, "y" => 0)));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              array("x" => 0, "y" => 0)));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              array("y" => 0)));
var_dump(imageaffinematrixget(IMG_AFFINE_SCALE,
                              array("x" => 0)));

var_dump(imageaffinematrixget(IMG_AFFINE_ROTATE, 1));
var_dump(imageaffinematrixget(IMG_AFFINE_SHEAR_HORIZONTAL, 1));
var_dump(imageaffinematrixget(IMG_AFFINE_SHEAR_VERTICAL, 1));
