<?hh
const int global_const = 1;
enum DebugFlag: int {
  ON = 1;
  OFF = 0;
}


//function tests
function return_same_int(bool $x): int {
  if ($x) {
    return 123;
  } else {
    return 123; //should produce lint
  }
}

function return_same_string(bool $x): string {
  if ($x) {
    return "hello";
  } else {
    return "hello"; //should produce lint
  }
}


function return_same_float(bool $x): float {
  if ($x) {
    return 1.5;
  } else {
    return 1.5; //should produce lint
  }
}

function return_same_bool(bool $x): bool {
  if ($x) {
    $y = 2;
    return true;
  } else {
    $y = 3;
    return true; //should produce lint
  }
}

function return_same_global_constant(bool $x): int {
  if ($x) {
    return global_const;
  } else {
    return global_const; //should produce lint
  }
}

function return_same_enum_constant(bool $x): DebugFlag {
  if ($x) {
    return DebugFlag::ON;
  } else {
    return DebugFlag::ON; //should produce lint
  }
}

function return_same_function_call(bool $x): int {
  if ($x) {
    return return_different_string($x);
  } else {
    return return_different_string($x); // should not produce lint
  }
}

function return_different_enum_constant(bool $x): DebugFlag {
  if ($x) {
    return DebugFlag::ON;
  } else {
    return DebugFlag::OFF; //should not produce lint
  }
}

function return_different_float(bool $x): float {
  if ($x) {
    return 1.0;
  } else {
    return 1.5; //should not produce lint
  }
}

function return_different_string(bool $x): string {
  if ($x) {
    return "hello1";
  } else {
    return "hello2"; //should not produce lint
  }
}

function return_while(bool $x): int {
  while ($x) {
    return 123;
  }
  return 123;
}

function return_same_string_else_if(bool $x): string {
  if ($x) {
    return "hello";
  } else if (!$x) {
    return "hello";
  } else {
    return "hello"; //should produce lint
  }
}

function return_different_string_else_if(bool $x): string {
  if ($x) {
    $y = 2;
    return "hello1";
  } else if (!$x) {
    return "hello";
  } else {
    $y = 3;
    return "hello1"; //should not produce lint
  }
}

function return_string_lambda_efun(string $x, bool $y): string {
  if ($y) {
    $y = 2;
    return "hello";
  }
  $lambda = function($x) {
    return "different";
  };
  return "hello"; //should produce lint
}

function return_string_lambda_lfun(string $x, bool $y): string {
  if ($y) {
    $y = 2;
    return "hello";
  }
  $lambda = ($x) ==> {
    return "different";
  };
  return "hello"; //should produce lint
}

function return_same_string_nested_if(bool $x): string {
  if ($x) {
    if ($x) {
      return "hello";
    }
    return "hello";
  } else if (!$x) {
    return "hello";
  } else {
    return "hello"; //should produce lint
  }
}

function return_different_string_nested_if(bool $x): string {
  if ($x) {
    if ($x) {
      return "hello1";
    }
    return "hello";
  } else if (!$x) {
    return "hello";
  } else {
    return "hello"; //should not produce lint
  }
}

//method tests

class Foo {
  public function returnSameInt(bool $x): int {
    if ($x) {
      return 123;
    } else {
      return 123; //should produce lint
    }
  }

  public function returnSameString(bool $x): string {
    if ($x) {
      return "hello";
    } else {
      return "hello"; //should produce lint
    }
  }


  public function returnSameFloat(bool $x): float {
    if ($x) {
      return 1.5;
    } else {
      return 1.5; //should produce lint
    }
  }

  public function returnSameBool(bool $x): bool {
    if ($x) {
      $y = 2;
      return true;
    } else {
      $y = 3;
      return true; //should produce lint
    }
  }

  public function returnSameGlobalConstant(bool $x): int {
    if ($x) {
      return global_const;
    } else {
      return global_const; //should produce lint
    }
  }

  public function returnSameEnumConstant(bool $x): DebugFlag {
    if ($x) {
      return DebugFlag::ON;
    } else {
      return DebugFlag::ON; //should produce lint
    }
  }

  function returnSameFunctionCall(bool $x): int {
    if ($x) {
      return this->returnDifferentString($x);
    } else {
      return this->returnDifferentString($x); // should not produce lint
    }
  }

  public function returnDifferentEnumConstant(bool $x): DebugFlag {
    if ($x) {
      return DebugFlag::ON;
    } else {
      return DebugFlag::OFF; //should not produce lint
    }
  }

  public function returnDifferentFloat(bool $x): float {
    if ($x) {
      return 1.0;
    } else {
      return 1.5; //should not produce lint
    }
  }

  public function returnDifferentString(bool $x): string {
    if ($x) {
      return "hello1";
    } else {
      return "hello2"; //should not produce lint
    }
  }

  public function returnSameStringElseIf(bool $x): string {
    if ($x) {
      return "hello";
    } else if (!$x) {
      return "hello";
    } else {
      return "hello"; //should produce lint
    }
  }

  public function returnDifferentStringElseIf(bool $x): string {
    if ($x) {
      $y = 2;
      return "hello1";
    } else if (!$x) {
      return "hello";
    } else {
      $y = 3;
      return "hello1"; //should not produce lint
    }
  }

  public function returnStringLambdaEfun(string $x, bool $y): string {
    if ($y) {
      $y = 2;
      return "hello";
    }
    $lambda = function($x) {
      return "different";
    };
    return "hello"; //should produce lint
  }

  public function returnStringLambdaLfun(string $x, bool $y): string {
    if ($y) {
      $y = 2;
      return "hello";
    }
    $lambda = ($x) ==> {
      return "different";
    };
    return "hello"; //should produce lint
  }

  public function returnWhile(bool $x): int {
    while ($x) {
      return 123;
    }
    return 123;
  }
  public function returnSameStringNestedIf(bool $x): string {
    if ($x) {
      if ($x) {
        return "hello";
      }
      return "hello";
    } else if (!$x) {
      return "hello";
    } else {
      return "hello"; //should produce lint
    }
  }

  public function returnDifferentStringNestedIf(bool $x): string {
    if ($x) {
      if ($x) {
        return "hello1";
      }
      return "hello";
    } else if (!$x) {
      return "hello";
    } else {
      return "hello"; //should not produce lint
    }
  }
}
