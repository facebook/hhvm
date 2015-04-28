<?hh

async function foo() {
  // Hide a bool in APC so static analysis can't see it.
  apc_store('mybool2', true);
  return apc_fetch('mybool2');
}

function returningFalse() {
  // Hide a bool in APC so static analysis can't see it.
  apc_store('mybool', false);
  return apc_fetch('mybool');
}

class :x:frag {}
class :ui:action-list extends :x:frag {}
class :ui:form extends :x:frag {}

final class foo {
  public async function genThings() {
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

  private async function genC() {
    $text = 'a';
    $primary = 'b';
    $secondary =
      <ui:action-list>
      </ui:action-list>;
    return tuple($text, $primary, $secondary);
  }

  private async function genB() {
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

  private async function genA() {
    $form =
      <ui:form>
      </ui:form>;
    return tuple($text, $form, <x:frag />);
  }
}

async function go() {
  $x = new foo;
  await $x->genThings();
  await $x->genThings();
  await $x->genThings();
  await $x->genThings();
  await $x->genThings();
}
$y = go();
HH\Asio\join($y);
