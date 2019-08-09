type a =
  [ `i
  | `j of int
  | `k of int * int
  ]

type b =
  [ a
  | `l
  ]
