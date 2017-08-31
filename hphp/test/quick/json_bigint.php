<?hh
var_dump(json_decode("8012345678901234567", true, 512, JSON_FB_LOOSE));
var_dump(json_decode("9012345678901234567", true, 512, JSON_FB_LOOSE));
var_dump(json_decode("80123456789012345678", true, 512, JSON_FB_LOOSE));
var_dump(json_decode("90123456789012345678", true, 512, JSON_FB_LOOSE));
