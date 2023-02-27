include module type of Hips2_intf

module Make (Intra : Intra) : T with type intra_constraint_ = Intra.constraint_
