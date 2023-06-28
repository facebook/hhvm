<?hh

class C {
  public function f() :mixed{
    try {}
    catch (Exception $a) {}
    catch (Exception $this) {}
  }
}

