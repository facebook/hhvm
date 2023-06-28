<?hh

async function foo() :Awaitable<mixed>{
  // Hide a bool in APC so static analysis can't see it.
  apc_store('mybool2', true);
  return __hhvm_intrinsics\apc_fetch_no_check('mybool2');
}

function returningFalse() :mixed{
  // Hide a bool in APC so static analysis can't see it.
  apc_store('mybool', false);
  return __hhvm_intrinsics\apc_fetch_no_check('mybool');
}

class :x:frag {}
class :ui:action-list extends :x:frag {}
class :ui:form extends :x:frag {}

final class foo {
  public async function genThings() :Awaitable<mixed>{
    if (returningFalse()) {
      $is_bouncing = await foo();
      if ($is_bouncing) {
        $gen = $this->genA();
      } else {
        $gen = $this->genB();
      }
    } else {
      $gen = $this->genC();
    }
    list($text, $primary, $secondary) = await $gen;
    return $secondary;
  }

  private async function genC() :Awaitable<mixed>{
    $text = 'a';
    $primary = 'b';
    $secondary =
      <ui:action-list>
      </ui:action-list>;
    return tuple($text, $primary, $secondary);
  }

  private async function genB() :Awaitable<mixed>{
    $text = 'asd';
    $primary = 'asd';
    if ($major_domain) {
      $secondary =
        <ui:action-list>
        </ui:action-list>;
    } else {
      $secondary =
        <ui:action-list>
        </ui:action-list>;
    }
    return tuple($text, $primary, $secondary);
  }

  private async function genA() :Awaitable<mixed>{
    $form =
      <ui:form>
      </ui:form>;
    return tuple($text, $form, <x:frag />);
  }
}

async function go() :Awaitable<mixed>{
  $x = new foo;
  await $x->genThings();
  await $x->genThings();
  await $x->genThings();
  await $x->genThings();
  await $x->genThings();
}

<<__EntryPoint>>
function main_async_await_002() :mixed{
$y = go();
HH\Asio\join($y);
}
