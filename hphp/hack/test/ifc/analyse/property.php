<?hh // strict

class Other {
  <<Policied("oBool")>>
  public bool $oBool = false;
  public string $oString = 'Hi there!';
}

class My {
  public function __construct(
    <<Policied("mInt")>>
    public int $mInt,
    <<Policied("other")>>
    public Other $other,
    public bool $mBool,
  ) { }

  public function getMInt(): int {
    return $this->mInt;
  }

  public function setMInt(int $candidate): void {
    $this->mInt = $candidate;
  }
}

function tlGetMyInt(My $obj): int {
  return $obj->mInt;
}

function tlSetMyInt(My $obj, int $val): void {
  $obj->mInt = $val;
}

function tlGetOther(My $obj): Other {
  return $obj->other;
}

function tlGetOtherBool(My $obj): bool {
  return $obj->other->oBool;
}

function tlSetOtherBool(My $obj, bool $bool): void {
  $obj->other->oBool = $bool;
}
