<?hh

<<__EntryPoint>>
function main_locale_lookup_two_params() :mixed{
$arr = vec[
    'de-DEVA',
    'de-DE-1996',
    'de',
    'de-De'
];
echo locale_lookup($arr, 'de-DE-1996-x-prv1-prv2');
}
