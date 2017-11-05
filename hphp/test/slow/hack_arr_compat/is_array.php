<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function normal($x) {
  var_dump(is_array(__hhvm_intrinsics\launder_value($x)));
}
function dynamic($x) {
  var_dump(__hhvm_intrinsics\launder_value('is_array')(__hhvm_intrinsics\launder_value($x)));
}

function literals() {
  var_dump(is_array(false));
  var_dump(is_array([]));
  var_dump(is_array(vec[]));
  var_dump(is_array(dict[]));
  var_dump(is_array(keyset[]));
  var_dump(is_array(varray[]));
  var_dump(is_array(darray[]));
}

echo "================= literals ===========================\n";
literals();

echo "================= normal =============================\n";

normal(false);
normal([]);
normal(vec[]);
normal(dict[]);
normal(keyset[]);
normal(varray[]);
normal(darray[]);

echo "================= dynamic =============================\n";

dynamic(false);
dynamic([]);
dynamic(vec[]);
dynamic(dict[]);
dynamic(keyset[]);
dynamic(varray[]);
dynamic(darray[]);
