<?hh

function returnsvoid() : void {}

function test() : void {

  $lam = (bool $b) ==> {
    if($b) {
      return returnsvoid();
    }
  };

}
