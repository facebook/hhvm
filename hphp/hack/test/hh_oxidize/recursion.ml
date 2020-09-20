type int_list =
  | Nil
  | Cons of int * int_list

type 'a n_ary_tree =
  | Leaf of 'a
  | Children of 'a n_ary_tree list
