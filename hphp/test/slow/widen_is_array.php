<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function normal_array($x) {
  var_dump(is_array(__hhvm_intrinsics\launder_value($x)));
}
function normal_varray($x) {
  var_dump(is_varray(__hhvm_intrinsics\launder_value($x)));
}
function normal_darray($x) {
  var_dump(is_darray(__hhvm_intrinsics\launder_value($x)));
}
function normal_vec($x) {
  var_dump(is_vec(__hhvm_intrinsics\launder_value($x)));
}
function normal_dict($x) {
  var_dump(is_dict(__hhvm_intrinsics\launder_value($x)));
}
function normal_php_array($x) {
  var_dump(HH\is_php_array(__hhvm_intrinsics\launder_value($x)));
}

function dynamic_array($x) {
  var_dump(__hhvm_intrinsics\launder_value('is_array')(__hhvm_intrinsics\launder_value($x)));
}
function dynamic_varray($x) {
  var_dump(__hhvm_intrinsics\launder_value('HH\is_varray')(__hhvm_intrinsics\launder_value($x)));
}
function dynamic_darray($x) {
  var_dump(__hhvm_intrinsics\launder_value('HH\is_darray')(__hhvm_intrinsics\launder_value($x)));
}
function dynamic_vec($x) {
  var_dump(__hhvm_intrinsics\launder_value('HH\is_vec')(__hhvm_intrinsics\launder_value($x)));
}
function dynamic_dict($x) {
  var_dump(__hhvm_intrinsics\launder_value('HH\is_dict')(__hhvm_intrinsics\launder_value($x)));
}
function dynamic_php_array($x) {
  var_dump(__hhvm_intrinsics\launder_value('HH\is_php_array')(__hhvm_intrinsics\launder_value($x)));
}

function literals() {
  echo "---------------------- is_array --------------------\n";

  var_dump(is_array(false));
  var_dump(is_array(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])));
  var_dump(is_array(vec[]));
  var_dump(is_array(dict[]));
  var_dump(is_array(keyset[]));
  var_dump(is_array(varray[]));
  var_dump(is_array(darray[]));

  echo "---------------------- is_varray -------------------\n";

  var_dump(is_varray(false));
  var_dump(is_varray(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])));
  var_dump(is_varray(vec[]));
  var_dump(is_varray(dict[]));
  var_dump(is_varray(keyset[]));
  var_dump(is_varray(varray[]));
  var_dump(is_varray(darray[]));

  echo "---------------------- is_darray -------------------\n";

  var_dump(is_darray(false));
  var_dump(is_darray(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])));
  var_dump(is_darray(vec[]));
  var_dump(is_darray(dict[]));
  var_dump(is_darray(keyset[]));
  var_dump(is_darray(varray[]));
  var_dump(is_darray(darray[]));

  echo "---------------------- is_vec ----------------------\n";

  var_dump(is_vec(false));
  var_dump(is_vec(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])));
  var_dump(is_vec(vec[]));
  var_dump(is_vec(dict[]));
  var_dump(is_vec(keyset[]));
  var_dump(is_vec(varray[]));
  var_dump(is_vec(darray[]));

  echo "---------------------- is_dict ---------------------\n";

  var_dump(is_dict(false));
  var_dump(is_dict(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])));
  var_dump(is_dict(vec[]));
  var_dump(is_dict(dict[]));
  var_dump(is_dict(keyset[]));
  var_dump(is_dict(varray[]));
  var_dump(is_dict(darray[]));

  echo "---------------------- is_php_array ----------------\n";

  var_dump(HH\is_php_array(false));
  var_dump(HH\is_php_array(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])));
  var_dump(HH\is_php_array(vec[]));
  var_dump(HH\is_php_array(dict[]));
  var_dump(HH\is_php_array(keyset[]));
  var_dump(HH\is_php_array(varray[]));
  var_dump(HH\is_php_array(darray[]));

}


<<__EntryPoint>>
function main_is_array(): void {
echo "================= literals ===========================\n";
literals();

echo "================= normal =============================\n";
echo "---------------------- is_array ----------------------\n";

normal_array(false);
normal_array(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
normal_array(vec[]);
normal_array(dict[]);
normal_array(keyset[]);
normal_array(varray[]);
normal_array(darray[]);

echo "---------------------- is_varray ---------------------\n";

normal_varray(false);
normal_varray(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
normal_varray(vec[]);
normal_varray(dict[]);
normal_varray(keyset[]);
normal_varray(varray[]);
normal_varray(darray[]);

echo "---------------------- is_darray ---------------------\n";

normal_darray(false);
normal_darray(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
normal_darray(vec[]);
normal_darray(dict[]);
normal_darray(keyset[]);
normal_darray(varray[]);
normal_darray(darray[]);

echo "---------------------- is_vec ------------------------\n";

normal_vec(false);
normal_vec(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
normal_vec(vec[]);
normal_vec(dict[]);
normal_vec(keyset[]);
normal_vec(varray[]);
normal_vec(darray[]);

echo "---------------------- is_dict -----------------------\n";

normal_dict(false);
normal_dict(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
normal_dict(vec[]);
normal_dict(dict[]);
normal_dict(keyset[]);
normal_dict(varray[]);
normal_dict(darray[]);

echo "---------------------- is_php_array ------------------\n";

normal_php_array(false);
normal_php_array(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
normal_php_array(vec[]);
normal_php_array(dict[]);
normal_php_array(keyset[]);
normal_php_array(varray[]);
normal_php_array(darray[]);

echo "================= dynamic =============================\n";
echo "---------------------- is_array ----------------------\n";

dynamic_array(false);
dynamic_array(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
dynamic_array(vec[]);
dynamic_array(dict[]);
dynamic_array(keyset[]);
dynamic_array(varray[]);
dynamic_array(darray[]);

echo "---------------------- is_varray ---------------------\n";

dynamic_varray(false);
dynamic_varray(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
dynamic_varray(vec[]);
dynamic_varray(dict[]);
dynamic_varray(keyset[]);
dynamic_varray(varray[]);
dynamic_varray(darray[]);

echo "---------------------- is_darray ---------------------\n";

dynamic_darray(false);
dynamic_darray(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
dynamic_darray(vec[]);
dynamic_darray(dict[]);
dynamic_darray(keyset[]);
dynamic_darray(varray[]);
dynamic_darray(darray[]);

echo "---------------------- is_vec ------------------------\n";

dynamic_vec(false);
dynamic_vec(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
dynamic_vec(vec[]);
dynamic_vec(dict[]);
dynamic_vec(keyset[]);
dynamic_vec(varray[]);
dynamic_vec(darray[]);

echo "---------------------- is_dict -----------------------\n";

dynamic_dict(false);
dynamic_dict(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
dynamic_dict(vec[]);
dynamic_dict(dict[]);
dynamic_dict(keyset[]);
dynamic_dict(varray[]);
dynamic_dict(darray[]);

echo "---------------------- is_php_array ------------------\n";

dynamic_php_array(false);
dynamic_php_array(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
dynamic_php_array(vec[]);
dynamic_php_array(dict[]);
dynamic_php_array(keyset[]);
dynamic_php_array(varray[]);
dynamic_php_array(darray[]);
}
