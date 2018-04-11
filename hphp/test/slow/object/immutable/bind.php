<?hh

class C {
  <<__Const>>
  public int $ci = 0;
  public int $i = 1;
}

function test_vget() {
  $c = new C();
  $li = 2;
  var_dump($c, $li);

  try {
    $li =& $c->ci;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $li =& $c->i;

  var_dump($c, $li);
}

function test_bind() {
  $c = new C();
  $li = 2;
  var_dump($c, $li);

  try {
    $c->ci =& $li;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $c->i =& $li;

  var_dump($c, $li);
}

test_vget();
test_bind();
