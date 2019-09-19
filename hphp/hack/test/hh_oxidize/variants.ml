type a =
  | I
  | J of int
  | K of int * int
  | L of (int * int)
  | M of {
      x: int;
      y: int;
    }
