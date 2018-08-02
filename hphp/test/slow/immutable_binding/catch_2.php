<?hh // experimental

class ExA extends Exception {}
class ExB extends Exception {}

function foo(bool $which): void {
  try {
    if ($which) {
      throw new ExA("A");
    } else {
      throw new ExB("B");
    }
  } catch (ExA e) {
    echo "Caught ExA ";
    var_dump(e->getMessage());
  } catch (ExB e) {
    echo "Caught ExB ";
    var_dump(e->getMessage());
  }
}

foo(true);
foo(false);
