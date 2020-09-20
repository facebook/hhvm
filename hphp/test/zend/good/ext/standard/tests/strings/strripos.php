<?hh <<__EntryPoint>> function main(): void {
var_dump(strripos("test test string", "test"));
var_dump(strripos("test string sTring", "string"));
var_dump(strripos("test strip string strand", "str"));
var_dump(strripos("I am what I am and that's all what I am", "am", -3));
var_dump(strripos("test string", "g"));
var_dump(strripos("te".chr(0)."st", chr(0)));
var_dump(strripos("tEst", "test"));
var_dump(strripos("teSt", "test"));
var_dump(@strripos("foo", "f", 1));
var_dump(@strripos("", ""));
var_dump(@strripos("a", ""));
var_dump(@strripos("", "a"));
var_dump(@strripos("\\\\a", "\\a"));
}
