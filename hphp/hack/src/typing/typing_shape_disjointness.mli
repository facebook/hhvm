(* (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary. *)

(** Every way the localized elements of a splat may fail to be pairwise
    disjoint. Field violations are grouped by label and retain every contributing
    position. Empty iff every pair is provably disjoint. *)
val violations :
  Typing_defs.locl_ty list ->
  Typing_env_types.env ->
  Typing_error.Primary.Shape_splat.disjointness_violation list

module For_test : sig
  type label_bound =
    | Bottom_plus of Typing_defs.TShapeSet.t
    | Top_minus of Typing_defs.TShapeSet.t

  val restrict :
    label_bound -> Typing_defs.TShapeSet.t -> Typing_defs.TShapeSet.t

  val disjoint : label_bound -> label_bound -> bool

  val join : label_bound -> label_bound -> label_bound

  val meet : label_bound -> label_bound -> label_bound

  type element

  val make_element :
    present:Typing_defs.TShapeSet.t -> bound:label_bound -> element

  val possible : element -> element

  val element_bound : element -> label_bound
end
