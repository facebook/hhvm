<?hh

class C {
  public function f() {
    try {}
    catch (Exception $a) {}
    catch (Exception $this) {}
  }
}

