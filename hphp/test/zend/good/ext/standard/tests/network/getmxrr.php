<?hh <<__EntryPoint>> function main(): void {
$domains = varray[ 'mx1.tests.php.net', 'mx2.tests.php.net' ];
foreach ( $domains as $domain )
{
    $hosts = null;
    $weights = null;
    if ( getmxrr( $domain, inout $hosts, inout $weights ) )
    {
        echo "Hosts: " . count( $hosts ) . ", weights: " . count( $weights ) . "\n";
    }
}
}
