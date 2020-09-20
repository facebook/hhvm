<?hh <<__EntryPoint>> function main(): void {
var_dump(0x80000000 * -0xffffffff);
var_dump(0x80000001 * 0xfffffffe);
var_dump(0x80000001 * -0xffffffff);
}
