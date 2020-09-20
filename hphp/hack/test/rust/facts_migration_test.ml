open Facts
open Hh_json

let validate_json_deser_ser json0 md5 sha1 =
  match facts_from_json json0 with
  | Some facts ->
    let json = facts_to_json ~md5 ~sha1 facts in
    if json0 = json then
      true
    else (
      Printf.printf "Actual: %s\n" (Hh_json.json_to_multiline json);
      false
    )
  | _ -> failwith "failed to deserialize Facts from JSON"

let test_json_deser_ser_idempotent () =
  let sha1 = "14c54bb46fa1562e2dd76ed1c2a98b59e32b5a44" in
  let md5 = "C0DEC0DEC0DEC0DEDEADBEEFDEADBEEF" in
  let json0 =
    JSON_Object
      [
        ("md5sum0", JSON_Number "-4548986510646525730");
        ("md5sum1", JSON_Number "-2401053088876216593");
        ("sha1sum", JSON_String sha1);
        ( "types",
          JSON_Array
            [
              JSON_Object
                [
                  ("name", JSON_String "Type1");
                  ("kindOf", JSON_String "trait");
                  ("flags", JSON_Number "6");
                  ( "baseTypes",
                    JSON_Array [JSON_String "Base1"; JSON_String "Base2"] );
                  ( "attributes",
                    JSON_Object
                      [("A", JSON_Array [JSON_String "B"; JSON_String "C"])] );
                  ("requireImplements", JSON_Array []);
                  ("requireExtends", JSON_Array []);
                ];
            ] );
        ("functions", JSON_Array [JSON_String "foo"; JSON_String "bar"]);
        ("constants", JSON_Array [JSON_String "c1"; JSON_String "c2"]);
        ("typeAliases", JSON_Array []);
      ]
  in
  validate_json_deser_ser json0 md5 sha1

let test_json_deser_ser_type_alias () =
  let sha1 = "82c78e3747fb89be849daf863a3fedf87788abb1" in
  let md5 = "C0DEC0DEC0DEC0DEDEADBEEFDEADBEEF" in
  let json0 =
    JSON_Object
      [
        ("md5sum0", JSON_Number "-4548986510646525730");
        ("md5sum1", JSON_Number "-2401053088876216593");
        ("sha1sum", JSON_String sha1);
        ( "types",
          JSON_Array
            [
              JSON_Object
                [
                  ("name", JSON_String "T0");
                  ("kindOf", JSON_String "typeAlias");
                  ("flags", JSON_Number "0");
                  ("baseTypes", JSON_Array []);
                  ("attributes", JSON_Object [("A", JSON_Array [])]);
                ];
            ] );
        ("functions", JSON_Array []);
        ("constants", JSON_Array []);
        ("typeAliases", JSON_Array [JSON_String "T0"]);
      ]
  in
  validate_json_deser_ser json0 md5 sha1

let () =
  Unit_test.run_all
    [
      ("test_json_deser_ser_idempotent", test_json_deser_ser_idempotent);
      ("test_json_deser_ser_type_alias", test_json_deser_ser_type_alias);
    ]
