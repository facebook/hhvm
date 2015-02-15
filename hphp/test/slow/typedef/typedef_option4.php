<?hh

type one = ?int;
type two = ?int;

function yeah(two $x) {
  var_dump($x);
}

yeah(12);
yeah(null);
yeah("fail"); // ends test
