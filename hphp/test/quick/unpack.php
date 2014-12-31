<?php

// 32-bit
var_dump(unpack("V", "\xFF\xFF\xFF\xFF"));
var_dump(unpack("N", "\xFF\xFF\xFF\xFF"));
var_dump(unpack("l", pack("l", -1)));
var_dump(unpack("V", pack("V", -1)));

// 64-bit
var_dump(unpack("P", "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"));
var_dump(unpack("J", "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"));
var_dump(unpack("q", pack("q", -1)));
var_dump(unpack("P", pack("P", -1)));

// Machine-Dependent
var_dump(unpack("i", "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"));
var_dump(unpack("I", "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"));
var_dump(unpack("i", pack("i", -1)));
var_dump(unpack("I", pack("I", -1)));
