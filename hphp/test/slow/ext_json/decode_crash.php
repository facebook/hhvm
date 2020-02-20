<?hh

var_dump(json_decode('"a"', false, 0, 0));
var_dump(json_decode('"abc', true, 1000, 0));
var_dump(json_decode('"\\u', true, 1000, 17180393472));
