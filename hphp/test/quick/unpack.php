<?hh
<<__EntryPoint>> function main(): void {
// 16-bit
var_dump(unpack("v", "\xFF\xFF"));
var_dump(unpack("n", "\xFF\xFF"));
var_dump(unpack("s", pack("s", -1)));
var_dump(unpack("v", pack("v", -1)));

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

// float
var_dump(unpack("f", "\x79\xE9\xF6\x42"));
var_dump(unpack("g", "\x79\xE9\xF6\x42"));
var_dump(unpack("G", "\x42\xF6\xE9\x79"));
var_dump(unpack("f", pack("f", 123.456)));
var_dump(unpack("g", pack("g", 123.456)));
var_dump(unpack("G", pack("G", 123.456)));

// double
var_dump(unpack("d", "\x77\xBE\x9F\x1A\x2F\xDD\x5E\x40"));
var_dump(unpack("e", "\x77\xBE\x9F\x1A\x2F\xDD\x5E\x40"));
var_dump(unpack("E", "\x40\x5E\xDD\x2F\x1A\x9F\xBE\x77"));
var_dump(unpack("d", pack("d", 123.456)));
var_dump(unpack("e", pack("e", 123.456)));
var_dump(unpack("E", pack("E", 123.456)));
}
