<?hh

<<__EntryPoint>> function main(): void {
for ( $i = -1050; $i <= -1000; $i++ )
{
    $M = "06";
    $D = "22";

    $dt = new DateTime("{$i}-{$M}-{$D} 00:00:00");
    $expected = "{$i}-{$M}-{$D} 00:00:00";
    $result = $dt->format('Y-m-d H:i:s');

    if ( $expected != $result )
    {
        echo "Wrong: Should have been {$expected}, was {$result}\n";
    }
}
echo "==DONE==";
}
