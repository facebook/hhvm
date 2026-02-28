<?hh

<<__EntryPoint>>
function main_duplicate_keys() :mixed{
var_dump(json_decode('{"a":"1","a":"2"}', true, 512, JSON_FB_LOOSE));
}
