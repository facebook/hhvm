module type S = sig
  type t

  val pp : t Fmt.t

  val to_string : t -> string

  val print : t -> unit
end

module type S1 = sig
  type _ t

  val pp : 'a Fmt.t -> 'a t Fmt.t

  val to_string : 'a Fmt.t -> 'a t -> string

  val print : 'a Fmt.t -> 'a t -> unit
end
