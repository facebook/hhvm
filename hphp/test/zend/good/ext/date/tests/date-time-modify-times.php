<?hh <<__EntryPoint>> function main(): void {
$tests = dict[
    '2010-12-15 19:42:45 UTC' => vec[
        'october 23:00', // October 23rd, with a broken time
        'back of 4pm',
        'next week monday',
        'next week monday 10am',
        'tuesday noon',
        'first monday of January 2011',
        'first monday of January 2011 09:00',
    ],
    '2010-12-15 19:42:45' => vec[
        'october 23:00', // October 23rd, with a broken time
        'march 28, 00:15',
        'march 28, 01:15', // doesn't exist bcause of DST
        'march 28, 02:15',
    ],
];

foreach ( $tests as $start => $data )
{
    foreach ( $data as $test )
    {
        echo date_create( $start )
            ->modify( $test )
            ->format( DateTime::RFC2822 ), "\n";
    }
}
echo "\n";
}
