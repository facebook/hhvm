<?hh

<<__EntryPoint>> function main(): void {
require_once(__DIR__ . '/test_base.inc');
init();
$requests = vec[
  vec[
    '-dalways_populate_raw_post_data=1',
    dict['CONTENT_TYPE' => 'multipart/form-data; boundary=dumy']],
  vec['-dalways_populate_raw_post_data=1', dict[]],
  vec['', dict[]],
];

foreach($requests as $request) {
  echo "------------ {$request[0]} --------\n";
  runTest(function($port) use($request) {
    list($options, $extra) = $request;
    $path = 'global_variables.php?var=GET&get=1';
    $post = dict['var' => 'POST', 'post' => 2];
    $headers = dict['Cookie' => 'var=COOKIE;cookie=3;'];
    echo fastcgi_request('localhost', $port, $path, $post, $headers, $extra) . "\n";
  }, $request[0]);
}
}
