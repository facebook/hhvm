<?hh

type foo = ?int;
type bar = foo;

function wat(bar $x) {
  var_dump($x);
}

wat(null);
wat(2);
wat("fail"); // ends test
