<?hh

class Other {
  <<__Policied("oBool")>>
  public bool $oBool = false;
  public string $oString = 'Hi there!';
}

class My {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("mInt")>>
    public int $mInt,
    <<__Policied("other")>>
    public Other $other,
    public bool $mBool,
  ) { }

  <<__InferFlows>>
  public function getMInt(): int {
    return $this->mInt;
  }

  <<__InferFlows>>
  public function setMInt(int $candidate): void {
    $this->mInt = $candidate;
  }
}

<<__InferFlows>>
function tlGetMyInt(My $obj): int {
  return $obj->mInt;
}

<<__InferFlows>>
function tlSetMyInt(My $obj, int $val): void {
  $obj->mInt = $val;
}

<<__InferFlows>>
function tlGetOther(My $obj): Other {
  return $obj->other;
}

<<__InferFlows>>
function tlGetOtherBool(My $obj): bool {
  return $obj->other->oBool;
}

<<__InferFlows>>
function tlSetOtherBool(My $obj, bool $bool): void {
  $obj->other->oBool = $bool;
}
