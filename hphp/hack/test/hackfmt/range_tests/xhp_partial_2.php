<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class :MyFancyXHPClass extends :OtherXHPThing {

  protected async function genXHP(): Awaitable<:xhp> {
    $button = <x:button use="primary">Click This Extra Extra Extra Long Text</x:button>;
    $link = <x:link href="#">Click This</x:link>;

    $clickable_sq =
      <div style="width: 100px; height: 100px; background: blue;" />;

    $ret =
      <x:column-layout-component
        attr1="attr"
        attr2={
          FakeClassWithReallyLongAttributeStuff::getCreateSomeStaticData(Map {
            'key1' => 'value1',
          })
        }>
        <x:div padding="large">
          <x:link href="#">Click Here!</x:link>
        </x:div>
        <x:div padding="large">
          {$link}
        </x:div>
        <x:div padding="large">
          <!--
            Here is a multiline comment that is very unhelpful. In fact, it is
            strange that it exists at all, let alone takes up multiple lines
          -->
          <x:form
            id="form"
            method="post"
            >
            <x:text-input
              name="input"
              placeholder="Type a message."
            />
            <x:button use="primary">Submit Form</x:button>
            <x:button use="other">This is a decoy button meant to confuse you</x:button>
          </x:form>
        </x:div>
      </x:column-layout-component>;

    PostXHPVariableSetLinesToCheckForPartialForamtting::go(Vector { 1, 2, 3, 4},  Vector {5, 6, 7 , 8});

    return $ret;
  }
}
