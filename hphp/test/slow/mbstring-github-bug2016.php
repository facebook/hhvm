<?hh


<<__EntryPoint>>
function main_mbstring_github_bug2016() :mixed{
var_dump(mb_substr('abc', 1, null, 'utf-8'));
var_dump(mb_strcut('def', 1, null, 'utf-8'));
}
