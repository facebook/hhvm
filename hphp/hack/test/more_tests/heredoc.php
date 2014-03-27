<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

$a = <<<END
This is a heredoc.
END;

print $a;

$interpolation = "an interpolated value";

$b = <<< END
This is a heredoc with $interpolation.
END;

$c = <<<END
   This one has whitespace and a misleading "end".
   END
END;

$d = <<<END
Words
Words
Words
Words
END;

class Test {
    public string $a;

    public function __construct() {
        $this->a = 'A';
    }

    public function __toString(): string {
        return '<Test>';
    }
}

$test = new Test();

print <<<identifier_with_underscores_and_like_50_characters
  $test = <Test>
  $test->a = a
  \x41 = A

identifier_with_underscores_and_like_50_characters;
