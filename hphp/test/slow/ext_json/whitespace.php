<?hh



<<__EntryPoint>>
function main_whitespace() :mixed{
// Valid whitespace.
var_dump(json_decode("[  42 ]", true, 512, JSON_FB_LOOSE));
var_dump(json_decode("\r[\n \t13 \r]", true, 512, JSON_FB_LOOSE));
// Following does not count as whitespace in JSON.
var_dump(json_decode("[\f5]", true, 512, JSON_FB_LOOSE));
var_dump(json_decode("[\v7]", true, 512, JSON_FB_LOOSE));
}
