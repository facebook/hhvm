<?hh


<<__EntryPoint>>
function main_query_string_prop() :mixed{
$rc = new ReflectionClass('PDOStatement');
$rp = $rc->getProperty('queryString');
echo $rp;
}
