<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function display($x) {
  $result = __hhvm_intrinsics\serialize_keep_dvarrays($x)[0];
  $lookup = dict[
    'a' => 'array',
    'y' => 'varray',
    'Y' => 'darray',
    'v' => 'vec',
    'D' => 'dict',
    'k' => 'keyset',
  ];
  return $lookup[$result] ?? 'non-array';
}

function normal_array($x) {
  return is_array(__hhvm_intrinsics\launder_value($x));
}
function normal_varray($x) {
  return is_varray(__hhvm_intrinsics\launder_value($x));
}
function normal_darray($x) {
  return is_darray(__hhvm_intrinsics\launder_value($x));
}
function normal_vec($x) {
  return is_vec(__hhvm_intrinsics\launder_value($x));
}
function normal_dict($x) {
  return is_dict(__hhvm_intrinsics\launder_value($x));
}
function normal_php_array($x) {
  return HH\is_php_array(__hhvm_intrinsics\launder_value($x));
}
function normal_any_array($x) {
  return HH\is_any_array(__hhvm_intrinsics\launder_value($x));
}

function dynamic_array($x) {
  return __hhvm_intrinsics\launder_value('is_array')(__hhvm_intrinsics\launder_value($x));
}
function dynamic_varray($x) {
  return __hhvm_intrinsics\launder_value('HH\is_varray')(__hhvm_intrinsics\launder_value($x));
}
function dynamic_darray($x) {
  return __hhvm_intrinsics\launder_value('HH\is_darray')(__hhvm_intrinsics\launder_value($x));
}
function dynamic_vec($x) {
  return __hhvm_intrinsics\launder_value('HH\is_vec')(__hhvm_intrinsics\launder_value($x));
}
function dynamic_dict($x) {
  return __hhvm_intrinsics\launder_value('HH\is_dict')(__hhvm_intrinsics\launder_value($x));
}
function dynamic_php_array($x) {
  return __hhvm_intrinsics\launder_value('HH\is_php_array')(__hhvm_intrinsics\launder_value($x));
}
function dynamic_any_array($x) {
  return __hhvm_intrinsics\launder_value('HH\is_any_array')(__hhvm_intrinsics\launder_value($x));
}

function literals() {
  echo "---------------------- is_array --------------------\n";

  printf("arrkind: %s, check: %d\n", display(false), is_array(false));
  printf("arrkind: %s, check: %d\n", display(vec[]), is_array(vec[]));
  printf("arrkind: %s, check: %d\n", display(dict[]), is_array(dict[]));
  printf("arrkind: %s, check: %d\n", display(keyset[]), is_array(keyset[]));
  printf("arrkind: %s, check: %d\n", display(varray[]), is_array(varray[]));
  printf("arrkind: %s, check: %d\n", display(darray[]), is_array(darray[]));

  echo "---------------------- is_varray -------------------\n";

  printf("arrkind: %s, check: %d\n", display(false), is_varray(false));
  printf("arrkind: %s, check: %d\n", display(vec[]), is_varray(vec[]));
  printf("arrkind: %s, check: %d\n", display(dict[]), is_varray(dict[]));
  printf("arrkind: %s, check: %d\n", display(keyset[]), is_varray(keyset[]));
  printf("arrkind: %s, check: %d\n", display(varray[]), is_varray(varray[]));
  printf("arrkind: %s, check: %d\n", display(darray[]), is_varray(darray[]));

  echo "---------------------- is_darray -------------------\n";

  printf("arrkind: %s, check: %d\n", display(false), is_darray(false));
  printf("arrkind: %s, check: %d\n", display(vec[]), is_darray(vec[]));
  printf("arrkind: %s, check: %d\n", display(dict[]), is_darray(dict[]));
  printf("arrkind: %s, check: %d\n", display(keyset[]), is_darray(keyset[]));
  printf("arrkind: %s, check: %d\n", display(varray[]), is_darray(varray[]));
  printf("arrkind: %s, check: %d\n", display(darray[]), is_darray(darray[]));

  echo "---------------------- is_vec ----------------------\n";

  printf("arrkind: %s, check: %d\n", display(false), is_vec(false));
  printf("arrkind: %s, check: %d\n", display(vec[]), is_vec(vec[]));
  printf("arrkind: %s, check: %d\n", display(dict[]), is_vec(dict[]));
  printf("arrkind: %s, check: %d\n", display(keyset[]), is_vec(keyset[]));
  printf("arrkind: %s, check: %d\n", display(varray[]), is_vec(varray[]));
  printf("arrkind: %s, check: %d\n", display(darray[]), is_vec(darray[]));

  echo "---------------------- is_dict ---------------------\n";

  printf("arrkind: %s, check: %d\n", display(false), is_dict(false));
  printf("arrkind: %s, check: %d\n", display(vec[]), is_dict(vec[]));
  printf("arrkind: %s, check: %d\n", display(dict[]), is_dict(dict[]));
  printf("arrkind: %s, check: %d\n", display(keyset[]), is_dict(keyset[]));
  printf("arrkind: %s, check: %d\n", display(varray[]), is_dict(varray[]));
  printf("arrkind: %s, check: %d\n", display(darray[]), is_dict(darray[]));

  echo "---------------------- is_php_array ----------------\n";

  printf("arrkind: %s, check: %d\n", display(false), HH\is_php_array(false));
  printf("arrkind: %s, check: %d\n", display(vec[]), HH\is_php_array(vec[]));
  printf("arrkind: %s, check: %d\n", display(dict[]), HH\is_php_array(dict[]));
  printf("arrkind: %s, check: %d\n", display(keyset[]), HH\is_php_array(keyset[]));
  printf("arrkind: %s, check: %d\n", display(varray[]), HH\is_php_array(varray[]));
  printf("arrkind: %s, check: %d\n", display(darray[]), HH\is_php_array(darray[]));

  echo "---------------------- is_any_array ----------------\n";

  printf("arrkind: %s, check: %d\n", display(false), HH\is_any_array(false));
  printf("arrkind: %s, check: %d\n", display(vec[]), HH\is_any_array(vec[]));
  printf("arrkind: %s, check: %d\n", display(dict[]), HH\is_any_array(dict[]));
  printf("arrkind: %s, check: %d\n", display(keyset[]), HH\is_any_array(keyset[]));
  printf("arrkind: %s, check: %d\n", display(varray[]), HH\is_any_array(varray[]));
  printf("arrkind: %s, check: %d\n", display(darray[]), HH\is_any_array(darray[]));

}


<<__EntryPoint>>
function main_is_array(): void {
echo "================= literals ===========================\n";
literals();

echo "================= normal =============================\n";
echo "---------------------- is_array ----------------------\n";

printf("arrkind: %s, check: %d\n", display(false), normal_array(false));
printf("arrkind: %s, check: %d\n", display(vec[]), normal_array(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), normal_array(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), normal_array(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), normal_array(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), normal_array(darray[]));

echo "---------------------- is_varray ---------------------\n";

printf("arrkind: %s, check: %d\n", display(false), normal_varray(false));
printf("arrkind: %s, check: %d\n", display(vec[]), normal_varray(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), normal_varray(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), normal_varray(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), normal_varray(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), normal_varray(darray[]));

echo "---------------------- is_darray ---------------------\n";

printf("arrkind: %s, check: %d\n", display(false), normal_darray(false));
printf("arrkind: %s, check: %d\n", display(vec[]), normal_darray(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), normal_darray(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), normal_darray(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), normal_darray(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), normal_darray(darray[]));

echo "---------------------- is_vec ------------------------\n";

printf("arrkind: %s, check: %d\n", display(false), normal_vec(false));
printf("arrkind: %s, check: %d\n", display(vec[]), normal_vec(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), normal_vec(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), normal_vec(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), normal_vec(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), normal_vec(darray[]));

echo "---------------------- is_dict -----------------------\n";

printf("arrkind: %s, check: %d\n", display(false), normal_dict(false));
printf("arrkind: %s, check: %d\n", display(vec[]), normal_dict(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), normal_dict(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), normal_dict(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), normal_dict(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), normal_dict(darray[]));

echo "---------------------- is_php_array ------------------\n";

printf("arrkind: %s, check: %d\n", display(false), normal_php_array(false));
printf("arrkind: %s, check: %d\n", display(vec[]), normal_php_array(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), normal_php_array(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), normal_php_array(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), normal_php_array(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), normal_php_array(darray[]));

echo "---------------------- is_any_array ------------------\n";

printf("arrkind: %s, check: %d\n", display(false), normal_any_array(false));
printf("arrkind: %s, check: %d\n", display(vec[]), normal_any_array(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), normal_any_array(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), normal_any_array(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), normal_any_array(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), normal_any_array(darray[]));

echo "================= dynamic =============================\n";
echo "---------------------- is_array ----------------------\n";

printf("arrkind: %s, check: %d\n", display(false), dynamic_array(false));
printf("arrkind: %s, check: %d\n", display(vec[]), dynamic_array(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), dynamic_array(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), dynamic_array(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), dynamic_array(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), dynamic_array(darray[]));

echo "---------------------- is_varray ---------------------\n";

printf("arrkind: %s, check: %d\n", display(false), dynamic_varray(false));
printf("arrkind: %s, check: %d\n", display(vec[]), dynamic_varray(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), dynamic_varray(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), dynamic_varray(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), dynamic_varray(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), dynamic_varray(darray[]));

echo "---------------------- is_darray ---------------------\n";

printf("arrkind: %s, check: %d\n", display(false), dynamic_darray(false));
printf("arrkind: %s, check: %d\n", display(vec[]), dynamic_darray(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), dynamic_darray(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), dynamic_darray(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), dynamic_darray(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), dynamic_darray(darray[]));

echo "---------------------- is_vec ------------------------\n";

printf("arrkind: %s, check: %d\n", display(false), dynamic_vec(false));
printf("arrkind: %s, check: %d\n", display(vec[]), dynamic_vec(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), dynamic_vec(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), dynamic_vec(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), dynamic_vec(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), dynamic_vec(darray[]));

echo "---------------------- is_dict -----------------------\n";

printf("arrkind: %s, check: %d\n", display(false), dynamic_dict(false));
printf("arrkind: %s, check: %d\n", display(vec[]), dynamic_dict(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), dynamic_dict(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), dynamic_dict(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), dynamic_dict(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), dynamic_dict(darray[]));

echo "---------------------- is_php_array ------------------\n";

printf("arrkind: %s, check: %d\n", display(false), dynamic_php_array(false));
printf("arrkind: %s, check: %d\n", display(vec[]), dynamic_php_array(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), dynamic_php_array(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), dynamic_php_array(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), dynamic_php_array(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), dynamic_php_array(darray[]));

echo "---------------------- is_any_array ------------------\n";

printf("arrkind: %s, check: %d\n", display(false), dynamic_any_array(false));
printf("arrkind: %s, check: %d\n", display(vec[]), dynamic_any_array(vec[]));
printf("arrkind: %s, check: %d\n", display(dict[]), dynamic_any_array(dict[]));
printf("arrkind: %s, check: %d\n", display(keyset[]), dynamic_any_array(keyset[]));
printf("arrkind: %s, check: %d\n", display(varray[]), dynamic_any_array(varray[]));
printf("arrkind: %s, check: %d\n", display(darray[]), dynamic_any_array(darray[]));
}
