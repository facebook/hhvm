<?hh

/** Class X is related to neither ParentClass nor ChildClass. */
class X {
  public static function test() :mixed{
    $myChild = new ChildClass;
    $myChild->secret(); // bug - invokes X::secret() instead of ChildClass::secret()
  }
  private function secret() :mixed{
    echo "Called private " . __METHOD__ . "() on an instance of: " . get_class($this) . "\n";
  }
}

class ParentClass {
  private function secret() :mixed{ }
}

class ChildClass extends ParentClass {
  public function secret() :mixed{
    echo "Called public " . __METHOD__ . "() on an instance of: " . get_class($this) . "\n";
  }
}

<<__EntryPoint>> function main(): void {
X::test();
}
