(** Structural validation for shape and tuple destructuring patterns.

    This check catches errors we do not catch in subtyping, such as:
    {[
      $s : shape('a' => int, 'b' => string);
      shape('a' => $a) = $s;            // error: missing required field 'b'

      $s : shape('a' => int, ...);
      shape('a' => $a) = $s;            // error: missing `...` for open shape

      $s : shape('a' => int);
      shape('a' => $a, 'z' => $z) = $s; // error: field 'z' doesn't exist

      $t : (int, string);
      tuple($a) = $t;                   // error: arity mismatch
    ]}

    See {!Typing_destructure} for the inference-time constraint emission
    that this check complements. *)
include Handler.S
