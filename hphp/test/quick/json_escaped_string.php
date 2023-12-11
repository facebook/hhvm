<?hh <<__EntryPoint>> function main(): void {
$data = vec[
  '"foo\nbar"',
  '"baz\u003dbong"',
];

foreach($data as $str) {
  var_dump(json_decode($str));
}
}
