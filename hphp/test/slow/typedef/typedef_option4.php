<?hh

type one = ?int;
type two = ?int;

function yeah(two $x) :mixed{
  var_dump($x);
}
<<__EntryPoint>> function main(): void {
yeah(12);
yeah(null);
yeah("fail"); // ends test
}
