<?hh


// originally based on ext/standard/tests/file/stream_rfc2397_006.phpt

<<__EntryPoint>>
function main_bad_data() :mixed{
$streams = vec[
  "data:;base64,\0Zm9vYmFyIGZvb2Jhcg==",
  "data:;base64,Zm9vYmFy\0IGZvb2Jhcg==",
  "data:;base64,Zm9vYmFyIGZvb2Jhcg==",
  'data:;base64,#Zm9vYmFyIGZvb2Jhcg==',
  'data:;base64,#Zm9vYmFyIGZvb2Jhc=',
];

foreach ($streams as $stream) {
  var_dump(file_get_contents($stream));
}
}
