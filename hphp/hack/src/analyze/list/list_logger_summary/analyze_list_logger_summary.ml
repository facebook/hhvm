(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let prefix = "@list_logger:"

let incr_hashtbl tbl key =
  let cur = Hashtbl.find tbl key |> Option.value ~default:0 in
  Hashtbl.set tbl ~key ~data:(cur + 1)

let sort_by_count_desc alist =
  List.sort alist ~compare:(fun (k1, a) (k2, b) ->
      let c = Int.compare b a in
      if c <> 0 then
        c
      else
        String.compare k1 k2)

(** Max examples to print per lvalue kind per category *)
let max_examples_per_kind = 5

(** Classify an entry into test / prod_generated / prod_nongenerated based on
    is_generated flag and filename path heuristics.
    Test takes priority: test code is test code regardless of @^generated status. *)
type source_category =
  | Test
  | ProdGenerated
  | ProdNongenerated

let classify_source (entry : Analyze_list_logger_types.t) : source_category =
  let filename = entry.pos.filename in
  if
    String.is_substring filename ~substring:"__tests__"
    || String.is_substring filename ~substring:"__tests/"
    || String.is_substring filename ~substring:"/tests/"
    || String.is_substring filename ~substring:"/test/"
  then
    Test
  else if entry.is_generated then
    ProdGenerated
  else
    ProdNongenerated

let category_name = function
  | Test -> "test"
  | ProdGenerated -> "prod_generated"
  | ProdNongenerated -> "prod_nongenerated"

(** Per-category stats *)
type category_stats = {
  mutable count: int;
  mutable complex_count: int;
  mutable total_lvalue_positions: int;
  mutable total_complex_elements: int;
  mutable call_involving_elements: int;
  bucket_counts: (string, int) Hashtbl.t;
  lvalue_kind_counts: (string, int) Hashtbl.t;
  lvalue_kind_examples: (string, (string * string * string) list) Hashtbl.t;
  receiver_kind_counts: (string, int) Hashtbl.t;
  misc_kind_counts: (string, int) Hashtbl.t;
  mutable complex_entries: (string * string * string) list;
}

let create_category_stats () =
  {
    count = 0;
    complex_count = 0;
    total_lvalue_positions = 0;
    total_complex_elements = 0;
    call_involving_elements = 0;
    bucket_counts = Hashtbl.create (module String);
    lvalue_kind_counts = Hashtbl.create (module String);
    lvalue_kind_examples = Hashtbl.create (module String) ~size:16;
    receiver_kind_counts = Hashtbl.create (module String);
    misc_kind_counts = Hashtbl.create (module String);
    complex_entries = [];
  }

(** Max complex entry examples to keep per category *)
let max_complex_entries = 50

let () =
  let input_channel =
    if Array.length Sys.argv > 1 then
      In_channel.create Sys.argv.(1)
    else
      In_channel.stdin
  in
  let total = ref 0 in
  let total_lvalue_positions = ref 0 in
  let total_complex_elements = ref 0 in
  let total_call_involving = ref 0 in
  let bucket_counts = Hashtbl.create (module String) in
  let complex_lvalue_count = ref 0 in
  let misc_kind_counts = Hashtbl.create (module String) in
  let lvalue_kind_counts = Hashtbl.create (module String) in
  let lvalue_kind_examples = Hashtbl.create (module String) ~size:16 in
  let receiver_kind_counts = Hashtbl.create (module String) in
  (* Per-category stats *)
  let cat_stats =
    [
      (Test, create_category_stats ());
      (ProdGenerated, create_category_stats ());
      (ProdNongenerated, create_category_stats ());
    ]
  in
  let get_cat_stats cat = List.Assoc.find_exn cat_stats ~equal:Poly.equal cat in
  (try
     while true do
       let line = In_channel.(input_line_exn input_channel) in
       match String.substr_index line ~pattern:prefix with
       | None -> ()
       | Some idx ->
         let json_str = String.drop_prefix line (idx + String.length prefix) in
         let entry = Analyze_list_logger_types.of_json_string json_str in
         incr total;
         let bucket_name = Analyze_list_logger_types.show_bucket entry.bucket in
         incr_hashtbl bucket_counts bucket_name;
         total_lvalue_positions :=
           !total_lvalue_positions + entry.total_lvalue_count;
         let num_complex = List.length entry.complex_lvalue_elements in
         total_complex_elements := !total_complex_elements + num_complex;
         if entry.has_complex_lvalue then incr complex_lvalue_count;
         (* Per-category tracking *)
         let cat = classify_source entry in
         let cs = get_cat_stats cat in
         cs.count <- cs.count + 1;
         cs.total_lvalue_positions <-
           cs.total_lvalue_positions + entry.total_lvalue_count;
         cs.total_complex_elements <- cs.total_complex_elements + num_complex;
         incr_hashtbl cs.bucket_counts bucket_name;
         if entry.has_complex_lvalue then begin
           cs.complex_count <- cs.complex_count + 1;
           (* Keep bounded examples of complex entries per category *)
           if List.length cs.complex_entries < max_complex_entries then
             cs.complex_entries <-
               ( entry.lhs_code,
                 Printf.sprintf "%s:%d" entry.pos.filename entry.pos.line,
                 entry.type_string )
               :: cs.complex_entries
         end;
         (* Collect complex lvalue element info (global and per-category) *)
         List.iter
           entry.complex_lvalue_elements
           ~f:(fun (elem : Analyze_list_logger_types.complex_lvalue_element) ->
             let kind_name =
               Analyze_list_logger_types.show_lvalue_kind elem.lvalue_kind
             in
             incr_hashtbl lvalue_kind_counts kind_name;
             incr_hashtbl cs.lvalue_kind_counts kind_name;
             if elem.lvalue_involves_call then begin
               incr total_call_involving;
               cs.call_involving_elements <- cs.call_involving_elements + 1
             end;
             (* Track receiver kind for Class_get elements *)
             (match elem.lvalue_receiver_kind with
             | Some rk ->
               let rk_name = Analyze_list_logger_types.show_receiver_kind rk in
               incr_hashtbl receiver_kind_counts rk_name;
               incr_hashtbl cs.receiver_kind_counts rk_name
             | None -> ());
             let add_example tbl =
               let examples =
                 Hashtbl.find tbl kind_name |> Option.value ~default:[]
               in
               if List.length examples < max_examples_per_kind then
                 Hashtbl.set
                   tbl
                   ~key:kind_name
                   ~data:
                     (( elem.lvalue_code,
                        Printf.sprintf
                          "%s:%d"
                          elem.lvalue_pos.filename
                          elem.lvalue_pos.line,
                        entry.lhs_code )
                     :: examples)
             in
             add_example lvalue_kind_examples;
             add_example cs.lvalue_kind_examples);
         (match entry.bucket with
         | Analyze_list_logger_types.Misc ->
           (match entry.ty_node_kind with
           | Some kind ->
             incr_hashtbl misc_kind_counts kind;
             incr_hashtbl cs.misc_kind_counts kind
           | None -> ())
         | _ -> ())
     done
   with
  | End_of_file -> ());
  if not (phys_equal input_channel In_channel.stdin) then
    In_channel.close input_channel;

  Printf.printf "=== List Logger Summary ===\n";
  Printf.printf "Total entries: %d\n" !total;
  Printf.printf "Total lvalue positions: %d\n" !total_lvalue_positions;
  Printf.printf "\n";

  Printf.printf "Bucket counts:\n";
  let sorted_buckets = Hashtbl.to_alist bucket_counts |> sort_by_count_desc in
  List.iter sorted_buckets ~f:(fun (bucket, count) ->
      let pct =
        if !total > 0 then
          Float.of_int count /. Float.of_int !total *. 100.0
        else
          0.0
      in
      Printf.printf "  %-20s %6d  (%5.1f%%)\n" bucket count pct);
  Printf.printf "\n";

  Printf.printf
    "Complex lvalue: %d entries (%5.1f%%), %d elements\n"
    !complex_lvalue_count
    (if !total > 0 then
      Float.of_int !complex_lvalue_count /. Float.of_int !total *. 100.0
    else
      0.0)
    !total_complex_elements;
  if !total_call_involving > 0 then
    Printf.printf
      "Complex elements involving function calls: %d\n"
      !total_call_involving;

  (* Complex lvalue breakdown with denominator = all lvalue positions *)
  if not (Hashtbl.is_empty lvalue_kind_counts) then begin
    Printf.printf "\n";
    Printf.printf
      "Complex lvalue breakdown by kind (of %d total lvalue positions):\n"
      !total_lvalue_positions;
    let sorted_kinds =
      Hashtbl.to_alist lvalue_kind_counts |> sort_by_count_desc
    in
    List.iter sorted_kinds ~f:(fun (kind, count) ->
        let pct =
          if !total_lvalue_positions > 0 then
            Float.of_int count /. Float.of_int !total_lvalue_positions *. 100.0
          else
            0.0
        in
        Printf.printf "  %-20s %6d  (%5.2f%%)\n" kind count pct;
        let examples =
          Hashtbl.find lvalue_kind_examples kind |> Option.value ~default:[]
        in
        List.iter (List.rev examples) ~f:(fun (code, loc, lhs) ->
            Printf.printf "    example: %s  (%s)  lhs: %s\n" code loc lhs));
    (* Global receiver kind breakdown *)
    let class_get_total =
      Hashtbl.find lvalue_kind_counts "Class_get" |> Option.value ~default:0
    in
    if not (Hashtbl.is_empty receiver_kind_counts) then begin
      Printf.printf "\n";
      Printf.printf "  Class_get receiver kind breakdown:\n";
      let sorted_rk =
        Hashtbl.to_alist receiver_kind_counts |> sort_by_count_desc
      in
      List.iter sorted_rk ~f:(fun (rk, count) ->
          let pct =
            if class_get_total > 0 then
              Float.of_int count /. Float.of_int class_get_total *. 100.0
            else
              0.0
          in
          Printf.printf "    %-20s %6d  (%5.1f%%)\n" rk count pct)
    end
  end;

  if not (Hashtbl.is_empty misc_kind_counts) then begin
    Printf.printf "\n";
    Printf.printf "Misc breakdown by ty_node_kind:\n";
    let sorted_kinds =
      Hashtbl.to_alist misc_kind_counts |> sort_by_count_desc
    in
    List.iter sorted_kinds ~f:(fun (kind, count) ->
        Printf.printf "  %-20s %6d\n" kind count)
  end;

  (* Per-category breakdown *)
  List.iter cat_stats ~f:(fun (cat, cs) ->
      if cs.count > 0 then begin
        Printf.printf "\n";
        Printf.printf
          "=== Category: %s (%d entries, %d lvalue positions) ===\n"
          (category_name cat)
          cs.count
          cs.total_lvalue_positions;
        Printf.printf "\n";
        Printf.printf "  Bucket counts:\n";
        let sorted = Hashtbl.to_alist cs.bucket_counts |> sort_by_count_desc in
        List.iter sorted ~f:(fun (bucket, count) ->
            let pct =
              if cs.count > 0 then
                Float.of_int count /. Float.of_int cs.count *. 100.0
              else
                0.0
            in
            Printf.printf "    %-20s %6d  (%5.1f%%)\n" bucket count pct);
        Printf.printf "\n";
        Printf.printf
          "  Complex lvalue: %d entries (%5.1f%%), %d elements\n"
          cs.complex_count
          (if cs.count > 0 then
            Float.of_int cs.complex_count /. Float.of_int cs.count *. 100.0
          else
            0.0)
          cs.total_complex_elements;
        if cs.call_involving_elements > 0 then
          Printf.printf
            "  Complex elements involving function calls: %d\n"
            cs.call_involving_elements;
        if not (Hashtbl.is_empty cs.lvalue_kind_counts) then begin
          Printf.printf "\n";
          Printf.printf
            "  Complex lvalue breakdown by kind (of %d lvalue positions):\n"
            cs.total_lvalue_positions;
          let sorted_kinds =
            Hashtbl.to_alist cs.lvalue_kind_counts |> sort_by_count_desc
          in
          List.iter sorted_kinds ~f:(fun (kind, count) ->
              let pct =
                if cs.total_lvalue_positions > 0 then
                  Float.of_int count
                  /. Float.of_int cs.total_lvalue_positions
                  *. 100.0
                else
                  0.0
              in
              Printf.printf "    %-20s %6d  (%5.2f%%)\n" kind count pct;
              let examples =
                Hashtbl.find cs.lvalue_kind_examples kind
                |> Option.value ~default:[]
              in
              List.iter (List.rev examples) ~f:(fun (code, loc, lhs) ->
                  Printf.printf
                    "      example: %s  (%s)  lhs: %s\n"
                    code
                    loc
                    lhs));
          (* Per-category receiver kind breakdown *)
          let cat_class_get_total =
            Hashtbl.find cs.lvalue_kind_counts "Class_get"
            |> Option.value ~default:0
          in
          if not (Hashtbl.is_empty cs.receiver_kind_counts) then begin
            Printf.printf "\n";
            Printf.printf "  Class_get receiver kind breakdown:\n";
            let sorted_rk =
              Hashtbl.to_alist cs.receiver_kind_counts |> sort_by_count_desc
            in
            List.iter sorted_rk ~f:(fun (rk, count) ->
                let pct =
                  if cat_class_get_total > 0 then
                    Float.of_int count
                    /. Float.of_int cat_class_get_total
                    *. 100.0
                  else
                    0.0
                in
                Printf.printf "    %-20s %6d  (%5.1f%%)\n" rk count pct)
          end
        end;
        if not (Hashtbl.is_empty cs.misc_kind_counts) then begin
          Printf.printf "\n";
          Printf.printf "  Misc breakdown by ty_node_kind:\n";
          let sorted_kinds =
            Hashtbl.to_alist cs.misc_kind_counts |> sort_by_count_desc
          in
          List.iter sorted_kinds ~f:(fun (kind, count) ->
              Printf.printf "    %-20s %6d\n" kind count)
        end;
        (* Complex entry examples *)
        if not (List.is_empty cs.complex_entries) then begin
          Printf.printf "\n";
          Printf.printf "  Complex entry examples:\n";
          List.iter (List.rev cs.complex_entries) ~f:(fun (lhs, loc, ty) ->
              Printf.printf "    %s = <%s>  (%s)\n" lhs ty loc)
        end
      end)
