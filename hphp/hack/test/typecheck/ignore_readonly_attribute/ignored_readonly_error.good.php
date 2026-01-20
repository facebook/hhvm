<?hh

class TypedMockFunction<Tfun> {
  <<__NoAutoLikes>>
  public function mockImplementationWithOriginal(
    <<__IgnoreReadonlyError>> (function(Tfun): Tfun) $callable,
  ): void {
  }
}

final class MockedTargetClass {
  public function method(int $a): int {
    return $a;
  }
}

class WWWTest {
  <<__NoAutoLikes>>
  final public static function mockFunction<Tfun>(
    HH\FunctionRef<Tfun> $fr,
  ): TypedMockFunction<Tfun> {
    return new TypedMockFunction();
  }
}

final class EmptyIntegrationTest extends WWWTest {
  final public async function testSimpleAssert(): Awaitable<void> {
    self::mockFunction(meth_caller(MockedTargetClass::class, 'method'))
      ->mockImplementationWithOriginal(
        ($original) ==> ($instance, int $a) ==> {
          if ($a === 5) {
            return $original($instance, 123);
          }
          return $a;
        },
      );
  }
}
