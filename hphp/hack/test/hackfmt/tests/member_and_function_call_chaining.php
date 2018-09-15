<?hh // strict

function member_and_function_call_chain(MyObject $my_object): void {
  $my_object
    ->getASubObjectFromMyObject()
    ->getSomeOtherObject()
    ->directObject
    ?->field
    ->subField
    ?->method();

  $my_object
    ?->getASubObjectFromMyObject
    ->getSomeOtherObject()
    ->directObject
    ?->field
    ->subField
    ->method();

  $my_object
    ?->getASubObjectFromMyObject()
    ->getSomeOtherObject()
    ->directObject
    ?->field
    ->subField;

  $my_object
    ?->getASubObjectFromMyObject
    ->getSomeOtherObject()
    ->directObject
    ?->field;
}
