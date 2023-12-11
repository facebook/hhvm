<?hh <<__EntryPoint>> function main(): void {
$array = dict[
    'region_id' => 1,
    'discipline' => 23,
    'degrees' => vec[],
    'country_id' => 27
];

$serialized = serialize($array);

$deflated = gzdeflate($serialized, 9);
$inflated = gzinflate($deflated);

echo strlen($inflated),"\n";
echo "Done";
}
