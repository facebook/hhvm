<?hh <<__EntryPoint>> function main(): void {
$ch = curl_init();

$opt = dict[
    CURLOPT_AUTOREFERER  => TRUE,
    CURLOPT_BINARYTRANSFER => TRUE
];

curl_setopt( $ch, CURLOPT_AUTOREFERER  , TRUE );

foreach( $opt as $option => $value ) {
    curl_setopt( $ch, $option, $value );
}

var_dump($opt); // with this bug, $opt[58] becomes NULL
}
