<?hh

class X {}  // not derived from Exception

class Except extends Exception
{
    public $prop = 0;
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  try {
      echo "L0: In try-block\n";

  //  throw 10;       // Diagnosed; good
  //  throw new X;    // Diagnosed; good
      throw new Exception();
  }
  //*/
  ///*
  catch (int $e) {        // Not diagnosed; bad
      echo "L0: In catch-block int\n";
  }
  //*/
  ///*
  catch (X $e) {      // Not diagnosed; bad
      echo "L0: In catch-block int\n";
  }
  //*/
  ///*
  catch (Exception $e) {
      echo "L0: In catch-block Exception\n";

  //  throw $e;
  //  throw new Exception;
  //  throw;      // can't re-throw current exception ala C#, C++
  }
  //*/
  /*
  finally {
      echo "L0: In finally-block\n";
  }
  */

  echo "Try catch byRef\n";

  $o = new Except();
  echo "\$o->prop = " . $o->prop . "\n";

  try
  {
      echo "In try-block\n";

      throw new Except();

      echo "End of try-block\n";
  }
  catch (Except /*&*/ $e)
  {
      echo "In catch-block Except\n";
      echo "\$e->prop = " . $e->prop . "\n";
      $e->prop = 999;
      echo "\$e->prop = " . $e->prop . "\n";
  }

  echo "\$o->prop = " . $o->prop . "\n";
}
