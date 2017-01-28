(**
 * Put common type definitions outside of the module type signature so it can
 * be included in both the .mli and .ml.
 *)
module Types = struct
  type options = {
    root : Path.t;
  }
  type t = unit
  type init_env = options
end

module type S = sig
  include module type of Types
  include Informant_sig.S with type t := t and type init_env := init_env
end
