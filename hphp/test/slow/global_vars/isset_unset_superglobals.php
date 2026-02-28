<?hh


<<__EntryPoint>>
function main_isset_unset_superglobals() :mixed{
var_dump(\HH\global_isset('_GET'));

\HH\global_set('_GET',  1);
\HH\global_get('_GET');

\HH\global_unset('_GET');
}
