<?

var_dump(unpack("V", "\xFF\xFF\xFF\xFF"));
var_dump(unpack("N", "\xFF\xFF\xFF\xFF"));
var_dump(unpack("l", pack("l", -1)));
var_dump(unpack("V", pack("V", -1)));
