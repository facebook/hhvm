<?hh

class MySplObjectStorage extends SplObjectStorage {
    public function getHash($obj) {
        return 2;
    }
}

class MySplObjectStorage2 extends SplObjectStorage {
    public function getHash($obj) {
        throw new Exception("foo");
        return "asd";
    }
}

class MySplObjectStorage3 extends SplObjectStorage {
    public function getHash($obj) {
        return "asd";
    }
}
<<__EntryPoint>>
function main_entry(): void {
  $s = new SplObjectStorage();
  $o1 = new Stdclass;
  $o2 = new Stdclass;
  $s[$o1] = "some_value\n";
  echo $s->offsetGet($o1);

  try {
      $s1 = new MySplObjectStorage;
      $s1[$o1] = "foo";
  } catch(Exception $e) {
      echo "caught\n";
  }

  try {
      $s2 = new MySplObjectStorage2;
      $s2[$o2] = "foo";
  } catch(Exception $e) {
      echo "caught\n";
  }

  $s3 = new MySplObjectStorage3;
  $s3[$o1] = $o1;
  var_dump($s3[$o1]);
  $s3[$o2] = $o2;

  var_dump($s3[$o1] === $s3[$o2]);

  echo "===DONE===\n";
}
