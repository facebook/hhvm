<?hh

class ExpectObj<T> {
  final public function toBeNull(
    ?Str\SprintfFormatString $msg = null,
    mixed ...$args
  )[]: void { }

  final public function toNotBeNull<Tu>(
    ?Str\SprintfFormatString $msg = null,
    mixed ...$args
  )[]: Tu where T = ?Tu {
    if ($this->obj is null) {
      assert false;
    }
    return $this->obj;
  }

  final public function __construct(
    protected T $obj;
  }

}

function expect<T>(T $obj): ExpectObj<T> {
  return new ExpectObj<T>();
}


function optValue(): ?string {
  return "hello";
}

function myTest() {
  expect(optValue())->AUTO332
}
