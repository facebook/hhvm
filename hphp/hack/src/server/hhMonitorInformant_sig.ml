(**
 * Put common type definitions outside of the module type signature so it can
 * be included in both the .mli and .ml.
 *)
module Types = struct
  type options = {
    root : Path.t;
    allow_subscriptions : bool;
    (** Disable the informant - use the dummy instead. *)
    use_dummy : bool;
  }
  type init_env = options
end

module type S = sig
  type t
  include module type of Types
  include Informant_sig.S with type t := t and type init_env := init_env
end
