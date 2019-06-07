<?hh


<<__EntryPoint>>
function main_query_string_prop() {
$rc = new ReflectionClass('PDOStatement');
$rp = $rc->getProperty('queryString');
echo $rp;
}
