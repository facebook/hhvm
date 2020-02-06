<?hh

require_once(__DIR__ . '/test_base.inc');
<<__EntryPoint>> function main(): void {
$requests = varray[
  varray[
    '-dalways_populate_raw_post_data=1',
    darray['CONTENT_TYPE' => 'multipart/form-data; boundary=dumy']],
  varray['-dalways_populate_raw_post_data=1', darray[]],
  varray['', darray[]],
  varray['-dvariables_order=NONE -drequest_order=', darray[]],
  varray['-dvariables_order=E -drequest_order=GPC', darray[]],
  varray['-dvariables_order=CGP -drequest_order=GP', darray[]],
  varray['-dvariables_order=GC -drequest_order=CG', darray[]],
  varray['-dvariables_order=GC -drequest_order=GC', darray[]],
  varray['-dvariables_order=GC -drequest_order=P', darray[]],
];

foreach($requests as $request) {
  echo "------------ {$request[0]} --------\n";
  runTest(function($port) use($request) {
    list($options, $extra) = $request;
    $path = 'global_variables.php?var=GET&get=1';
    $post = darray['var' => 'POST', 'post' => 2];
    $headers = darray['Cookie' => 'var=COOKIE;cookie=3;'];
    echo request('localhost', $port, $path, $post, $headers, $extra) . "\n";
  }, $request[0]);
}
}
