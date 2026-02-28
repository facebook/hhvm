<?hh
<<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');
$formats = vec[
    '2017-03-25 10:52:09',
    '2017-03-25 10:52',
    '2017-03-25 10am',
    '2017-03-25',
    '2017-03',
    '2017.042',
    '2017043',
];

foreach ( $formats as $format )
{
    $dt = new DateTimeImmutable( $format );
    echo $dt->format( 'Y-m-d H:i:s' ), "\n";
}
}
