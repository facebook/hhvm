<?hh

function block() { return RescheduleWaitHandle::create(1,1); };

class F implements Awaitable {
  function __construct(){
    $this->val = 1;
  }

  function getWaitHandle() {
    $a = async function(){
      return $this->val;
    };
    return $a();
  }
}

async function foo() {
  $f = new F;
  $a = await $f;
  var_dump($a);
  await block();
  $f = new F;
  $a = await $f;
  var_dump($a);
}

foo()->join();
