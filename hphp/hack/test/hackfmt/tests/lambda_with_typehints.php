<?hh

class Foo {
  public function foo() {
    if (true) {
      if (true) {
        $some_query = $some_query->whereAsync(P::asyncLambda((
          SomeEntityType $entity,
        ) ==> $entity->genIsAvailableInLanguage($language)));
      }
    }
  }
}
