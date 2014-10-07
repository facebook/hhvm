open Hh_json

module CL = Coverage_level

let to_json r_opt =
  let json = match r_opt with
  | Some r -> JAssoc [
      "counter", JAssoc (List.map
        (fun (k, v) -> CL.string k, JInt v) r.CL.counter);
      "percentage", JFloat r.CL.percentage;
    ]
  | None -> JAssoc [ "internal_error", JBool true ]
  in json_to_string json
