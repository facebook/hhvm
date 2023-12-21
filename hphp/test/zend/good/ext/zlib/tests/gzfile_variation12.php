<?hh

<<__EntryPoint>> function main(): void {
$filename = dirname(__FILE__)."/004.txt.gz";


$variation = dict[
    'int 0' => 0,
    'int 1' => 1,
    'int 12345' => 12345,
    'int -12345' => -2345,
    ];


foreach ( $variation as $var ) {
  var_dump(gzfile( $filename, $var  ) );
}
echo "===DONE===\n";
}
