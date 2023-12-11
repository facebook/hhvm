<?hh <<__EntryPoint>> function main(): void {
$map = vec[0, 0x10FFFF, 0, 0xFFFFFF];
var_dump(mb_decode_numericentity('&', $map, 'UTF-8'));
var_dump(mb_decode_numericentity('&&&', $map, 'UTF-8'));
var_dump(mb_decode_numericentity('&#', $map, 'UTF-8'));
var_dump(mb_decode_numericentity('&#x', $map, 'UTF-8'));
var_dump(mb_decode_numericentity('&#61', $map, 'UTF-8'));
var_dump(mb_decode_numericentity('&#x3d', $map, 'UTF-8'));
var_dump(mb_decode_numericentity('&#61;', $map, 'UTF-8'));
var_dump(mb_decode_numericentity('&#x3d;', $map, 'UTF-8'));
}
