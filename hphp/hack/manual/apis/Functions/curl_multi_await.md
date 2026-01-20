
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The async equivalent to
[` curl_multi_select `](<http://php.net/manual/en/function.curl-multi-select.php>)




``` Hack
function curl_multi_await(
  resource $mh,
  float $timeout = 1,
): Awaitable<int>;
```




This function waits until there is activity on a cURL handle within ` $mh `.
Once there is activity, you process the result with
[` curl_multi_exec `](<http://php.net/manual/en/function.curl-multi-exec.php>)




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions




See Curl_exec() Wrt NoFCallBuiltin.](/hack/asynchronous-operations/extensions




See curl_exec() wrt NoFCallBuiltin.)







## Parameters




* ` resource $mh ` - A cURL multi handle returned from
  [` curl_multi_init `](<http://php.net/manual/en/function.curl-multi-init.php>).
* ` float $timeout = 1 ` - The time to wait for a response indicating some activity.




## Returns




- [` Awaitable<int> `](/apis/Classes/HH/Awaitable/) - - An [` Awaitable `](/apis/Classes/HH/Awaitable/) representing the `` int `` result of the
  activity. If returned ``` int ``` is positive, that
  represents the number of handles on which there
  was activity. If ```` 0 ````, that means no activity
  occurred. If negative, then there was a select
  failure.




## Examples




The following shows a scenario where you are going to wait for and return the result of activity on multiple curl handles. A bit of a simpler approach would be to use [` HH\Asio\curl_exec `](<//hack/reference/function/HH.Asio.curl_exec/>), which is a wrapper around `` curl_multi_await ``.




``` basic-usage.hack
async function get_curl_content(Set<string> $urls): Awaitable<Vector<string>> {

  $chs = Vector {};
  foreach ($urls as $url) {
    $ch = \curl_init($url);
    \curl_setopt($ch, \CURLOPT_RETURNTRANSFER, true);
    $chs[] = $ch;
  }

  $mh = \curl_multi_init();
  foreach ($chs as $ch) {
    \curl_multi_add_handle($mh, $ch);
  }

  $active = -1;
  do {
    $ret = \curl_multi_exec($mh, inout $active);
  } while ($ret == \CURLM_CALL_MULTI_PERFORM);

  while ($active && $ret == \CURLM_OK) {
    $select = await \curl_multi_await($mh);
    if ($select === -1) {
      // https://bugs.php.net/bug.php?id=61141
      await \HH\Asio\usleep(100);
    }
    do {
      $ret = \curl_multi_exec($mh, inout $active);
    } while ($ret == \CURLM_CALL_MULTI_PERFORM);
  }

  $content = Vector {};

  foreach ($chs as $ch) {
    $str = (string)\curl_multi_getcontent($ch);
    $content[] = \substr($str, 0, 10);
    \curl_multi_remove_handle($mh, $ch);
  }

  \curl_multi_close($mh);

  return $content;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $urls = Set {
    'https://hhvm.com/blog/2020/05/04/hhvm-4.56.html',
    'https://hhvm.com/blog/2020/10/21/hhvm-4.80.html',
  };
  $content = await get_curl_content($urls);
  \var_dump($content);
}
```
<!-- HHAPIDOC -->
