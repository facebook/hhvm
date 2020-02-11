<?hh <<__EntryPoint>> function main(): void {
$array = darray[
    'region_id' => 1,
    'discipline' => 23,
    'degrees' => varray[],
    'country_id' => 27
];

$serialized = serialize($array);

$deflated = gzdeflate($serialized, 9);
$inflated = gzinflate($deflated);

echo strlen($inflated),"\n";
echo "Done";
}
