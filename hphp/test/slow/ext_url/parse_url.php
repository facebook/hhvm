<?hh

<<__EntryPoint>>
function main_parse_url() :mixed{
var_dump(parse_url('http://www.facebook.com'));
var_dump(parse_url('https://lol:12345@www.facebook.com:14159/hhvmabcd'));
var_dump(parse_url('irc://chat.freenode.net/#hhvm'));
var_dump(parse_url('content/:/\*'));
var_dump(parse_url("//example.org:8088/sites/default/files/drums.mp3"));
var_dump(parse_url("http://xx23124:__ffdfdef__@www.test.com:12345/dir"));
var_dump(parse_url('127.0.0.1'));
var_dump(parse_url('127.0.0.1:1234'));
}
