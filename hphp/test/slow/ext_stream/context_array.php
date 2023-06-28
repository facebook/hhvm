<?hh


<<__EntryPoint>>
function main_context_array() :mixed{
$opts = darray[
  'http' => darray[
    'header' => varray[
      "Accept-Encoding: gzip",
      "User-Agent: Composer/source PHP 5.5.99)",
    ],
    'tls' => darray[
      "verify_peer_name"=>TRUE
    ],
  ],
];

$context = stream_context_create($opts);
var_dump(stream_context_get_params($context));
}
