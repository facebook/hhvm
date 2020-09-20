<?hh

class B { public function __wakeup() { var_dump('a'); } }
<<__EntryPoint>> function main(): void {
$serialized_strs = varray[
  'O:1:"B":0:{}',
  'V:1:"B":0:{}',
  'K:1:"B":0:{}',
  serialize(new \HH\Set()),
  serialize(new \HH\Map()),
];

foreach ($serialized_strs as $serialized_str) {
  var_dump(unserialize($serialized_str, darray['allowed_classes' => false]));
  var_dump(unserialize($serialized_str));
}
}
