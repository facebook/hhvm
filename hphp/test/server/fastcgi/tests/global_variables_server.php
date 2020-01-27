<?hh

require_once(__DIR__ . '/test_base.inc');
<<__EntryPoint>> function main(): void {
$requests = array(
  array(
    '-dalways_populate_raw_post_data=1',
    darray['CONTENT_TYPE' => 'multipart/form-data; boundary=dumy']),
  array('-dalways_populate_raw_post_data=1', darray[]),
  array('', darray[]),
  array('-dvariables_order=NONE -drequest_order=', darray[]),
  array('-dvariables_order=E -drequest_order=GPC', darray[]),
  array('-dvariables_order=CGP -drequest_order=GP', darray[]),
  array('-dvariables_order=GC -drequest_order=CG', darray[]),
  array('-dvariables_order=GC -drequest_order=GC', darray[]),
  array('-dvariables_order=GC -drequest_order=P', darray[]),
);

foreach($requests as $request) {
  echo "------------ {$request[0]} --------\n";
  runTest(function($port) use($request) {
    list($options, $extra) = $request;
    $path = 'global_variables.php?var=GET&get=1';
    $post = array('var' => 'POST', 'post' => 2);
    $headers = array('Cookie' => 'var=COOKIE;cookie=3;');
    echo request('localhost', $port, $path, $post, $headers, $extra) . "\n";
  }, $request[0]);
}
}
