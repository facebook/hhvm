<?hh
<<__EntryPoint>>
function main() :mixed{
$get = \HH\global_get('_GET');
parse_str("id=f03_photos&pgurl=http%3A//fifaworldcup.yahoo.com/03/en/photozone/index.html", inout $get);
\HH\global_set('_GET', $get);
\HH\global_set('_REQUEST', array_merge(\HH\global_get('_REQUEST'), \HH\global_get('_GET')));

echo \HH\global_get('_GET')['id'];
echo "\n";
echo \HH\global_get('_GET')['pgurl'];
echo "\n";
echo \HH\global_get('_REQUEST')['id'];
echo "\n";
echo \HH\global_get('_REQUEST')['pgurl'];
}
