<?hh
class JustAClass {
  function isEnabled(): bool {
    $callable = () ==> {
      return true;
    };
    return $callable();
  }
}
