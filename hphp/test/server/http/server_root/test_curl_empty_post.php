<?hh

<<__EntryPoint>>
function main_test_curl_empty_post() :mixed{
  $port = $_ENV['SERVERPORT'];
  $host = php_uname('n');
  $url = "http://$host:$port/hello.php";

  regular_curl_empty_post($url);
  multi_curl_empty_post($url);
}

function regular_curl_empty_post($url) :mixed{
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_exec($ch);
  curl_close($ch);
}

function multi_curl_empty_post($url) :mixed{
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_POST, 1);

  $mh = curl_multi_init();

  curl_multi_add_handle($mh,$ch);

  $active = 0;
  do {
    $mrc = curl_multi_exec($mh, inout $active);
  } while ($mrc == CURLM_CALL_MULTI_PERFORM);

  $ret = "";
  while ($active && $mrc == CURLM_OK) {
    if (curl_multi_select($mh) != -1) {
      do {
        try {
          $mrc = curl_multi_exec($mh, inout $active);
        } catch (Exception $e) {
          $ret .= ":::Exception: " . $e->getMessage() . "\n";
        }
      } while ($mrc == CURLM_CALL_MULTI_PERFORM);
    }
  }

  curl_multi_close($mh);
  echo $ret;
}
