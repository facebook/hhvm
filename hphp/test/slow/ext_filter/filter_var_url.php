<?hh

<<__EntryPoint>>
function main_filter_var_url() :mixed{
var_dump(filter_var('www.localtest.com', FILTER_VALIDATE_IP | FILTER_FLAG_IPV6));
}
