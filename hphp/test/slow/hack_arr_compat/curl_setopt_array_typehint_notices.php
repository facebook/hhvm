<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_curl_setopt_array_typehint_notices() :mixed{
  $handle = curl_init();
  echo "before\n";
  curl_setopt_array($handle, darray[CURLOPT_URL => 'http://localhost']);
  echo "after\n";
}
