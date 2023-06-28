<?hh

function main() :mixed{
  var_dump(
    curl_setopt(
      curl_init(),
      CURLOPT_STDERR,
      fopen('php://temp', 'r+')
    )
  );
}


<<__EntryPoint>>
function main_set_stderr_to_tempfile() :mixed{
main();
}
