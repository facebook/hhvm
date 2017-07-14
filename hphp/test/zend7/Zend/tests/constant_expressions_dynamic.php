<?php

const C_0 = 0;
const C_1 = 1;
const C_foo = "foo";
const C_arr = [0 => 0, "foo" => "foo"];

const T_1 = C_1 | 2;
const T_2 = C_1 . "foo";
const T_3 = C_1 > 1;
const T_4 = C_1 >= 1;
const T_5 = -C_1;
const T_6 = +C_1;
const T_7 = +C_foo;
const T_8 = !C_1;
const T_9 = C_0 || 0;
const T_10 = C_1 || 0;
const T_11 = C_0 && 1;
const T_12 = C_1 && 1;
const T_13 = C_0 ? "foo" : "bar";
const T_14 = C_1 ? "foo" : "bar";
const T_15 = C_0 ?: "bar";
const T_16 = C_1 ?: "bar";
const T_17 = C_arr[0];
const T_18 = C_arr["foo"];
const T_19 = [
    C_0,
    "foo" => "foo",
    42 => 42,
    3.14 => 3.14,
    null => null,
    false => false,
    true => true,
];
eval("const T_20x = 'a';");
const T_20 = null ?: (T_20x . 'bc');

var_dump(
    T_1, T_2, T_3, T_4, T_5, T_6, T_7, T_8, T_9, T_10,
    T_11, T_12, T_13, T_14, T_15, T_16, T_17, T_18, T_19, T_20
);

?>
