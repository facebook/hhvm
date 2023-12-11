<?hh

class Foo {
  public $bar;
  public $bling;
}
<<__EntryPoint>> function main(): void {
$data = vec[
  's:5:"Hello"',
  's:5:"Hello";',
  's:5:"Hello"Z',
  's:5:"Hello"ZZZZZ',
  'a:2:{i:0;s:5:"hello";i:1;s:5:"world";}',
  'a:2:{i:0;s:5:"hello";i:1;s:5:"world"X}',
  'a:2:{i:0;s:5:"hello"Xi:1;s:5:"world";}',
  'O:3:"Foo":2:{s:3:"bar";s:3:"baz";s:5:"bling";s:5:"blong";}',
  'O:3:"Foo":2:{s:3:"bar";s:3:"baz";s:5:"bling";s:5:"blong"X}',
  'O:3:"Foo":2:{s:3:"bar";s:3:"baz"Xs:5:"bling";s:5:"blong";}',
];

foreach($data as $serialized) {
  var_dump(unserialize($serialized));
}
}
