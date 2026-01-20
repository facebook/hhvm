
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A convenience wrapper around
[` curl_multi_await `](/apis/Functions/curl_multi_await/)




``` Hack
namespace HH\Asio;

function curl_exec(
  mixed $url_or_handle,
): Awaitable<string>;
```




Pass a cURL handle, or, more simply, a string containing a URL (and the
cURL handle will be created for you), and the cURL request will be executed
via async and the ` string ` result will be returned.




curl_multi_info_read must be used to retrieve error information,
curl_errno can't be used as this function is a wrapper to curl_multi_await.




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Parameters




* ` mixed $url_or_handle `




## Returns




- [` Awaitable<string> `](/apis/Classes/HH/Awaitable/) - - An [` Awaitable `](/apis/Classes/HH/Awaitable/) representing the `` string `` result
  of the cURL request.




## Examples




The following shows a scenario where you are going to wait for and return the result of cURL activity on URLs, using the convenient wrapper that is ` curl_exec `.




``` basic-usage.hack
async function get_curl_content(Set<string> $urls): Awaitable<Vector<string>> {
  $content = Vector {};
  foreach ($urls as $url) {
    $str = await \HH\Asio\curl_exec($url);
    $content[] = \substr($str, 0, 10);
  }
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
