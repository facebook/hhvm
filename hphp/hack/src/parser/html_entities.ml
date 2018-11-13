(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * HTML5 special entity decoding
 *
 * HHVM decodes certain HTML entities present in input strings before
 * generating bytecode. In order to generate bytecode identical to HHVM's,
 * this module performs the same HTML entity decoding as HHVM.
 * Mimics: zend-html.cpp
 * The list of entities tested was taken from
 * https://dev.w3.org/html5/html-author/charref on 09/27/2017.
 *)

open Core_kernel

module B = Bytes

let entity_regex = Str.regexp "&[^;]+;"

type html_entity_map = {
  start_char: int;
  table: string list
}

let set pos n b =
  B.set b pos (char_of_int n);
  b

let utf32_to_utf8 k =
  if k < 0x80 then begin
    (* buf[0] = k; *)
    B.create 1
    |> set 0 k
  end
  else if k < 0x800 then begin
  (* buf[0] = 0xc0 | (k >> 6);
     buf[1] = 0x80 | (k & 0x3f); *)
    B.create 2
    |> set 0 (0xc0 lor (k lsr 6))
    |> set 1 (0x80 lor (k land 0x3f))
  end
  else if k < 0x10000 then begin
  (* buf[0] = 0xe0 | (k >> 12);
     buf[1] = 0x80 | ((k >> 6) & 0x3f);
     buf[2] = 0x80 | (k & 0x3f); *)
    B.create 3
    |> set 0 (0xe0 lor (k lsr 12))
    |> set 1 (0x80 lor ((k lsr 6) land 0x3f))
    |> set 2 (0x80 lor (k land 0x3f))
  end
  else if k < 0x200000 then begin
  (* buf[0] = 0xf0 | (k >> 18);
     buf[1] = 0x80 | ((k >> 12) & 0x3f);
     buf[2] = 0x80 | ((k >> 6) & 0x3f);
     buf[3] = 0x80 | (k & 0x3f); *)
    B.create 4
    |> set 0 (0xf0 lor (k lsr 18))
    |> set 1 (0x80 lor ((k lsr 12) land 0x3f))
    |> set 2 (0x80 lor ((k lsr 6) land 0x3f))
    |> set 3 (0x80 lor (k land 0x3f))
  end
  else if k < 0x4000000 then begin
  (* buf[0] = 0xf8 | (k >> 24);
     buf[1] = 0x80 | ((k >> 18) & 0x3f);
     buf[2] = 0x80 | ((k >> 12) & 0x3f);
     buf[3] = 0x80 | ((k >> 6) & 0x3f);
     buf[4] = 0x80 | (k & 0x3f); *)
    B.create 5
    |> set 0 (0xf8 lor (k lsr 24))
    |> set 1 (0x80 lor ((k lsr 18) land 0x3f))
    |> set 2 (0x80 lor ((k lsr 12) land 0x3f))
    |> set 3 (0x80 lor ((k lsr 6) land 0x3f))
    |> set 4 (0x80 lor (k land 0x3f))
  end
  else begin
  (* buf[0] = 0xfc | (k >> 30);
     buf[1] = 0x80 | ((k >> 24) & 0x3f);
     buf[2] = 0x80 | ((k >> 18) & 0x3f);
     buf[3] = 0x80 | ((k >> 12) & 0x3f);
     buf[4] = 0x80 | ((k >> 6) & 0x3f);
     buf[5] = 0x80 | (k & 0x3f); *)
    B.create 6
    |> set 0 (0xfc lor (k lsr 30))
    |> set 1 (0x80 lor ((k lsr 24) land 0x3f))
    |> set 2 (0x80 lor ((k lsr 18) land 0x3f))
    |> set 3 (0x80 lor ((k lsr 12) land 0x3f))
    |> set 4 (0x80 lor ((k lsr 6) land 0x3f))
    |> set 5 (0x80 lor (k land 0x3f))
  end

let utf32_to_utf8 k =
  Bytes.to_string (utf32_to_utf8 k)

let decode_table =
  let ent_iso_8859_1 = [
    "nbsp"; "iexcl"; "cent"; "pound"; "curren"; "yen"; "brvbar";
    "sect"; "uml"; "copy"; "ordf"; "laquo"; "not"; "shy"; "reg";
    "macr"; "deg"; "plusmn"; "sup2"; "sup3"; "acute"; "micro";
    "para"; "middot"; "cedil"; "sup1"; "ordm"; "raquo"; "frac14";
    "frac12"; "frac34"; "iquest"; "Agrave"; "Aacute"; "Acirc";
    "Atilde"; "Auml"; "Aring"; "AElig"; "Ccedil"; "Egrave";
    "Eacute"; "Ecirc"; "Euml"; "Igrave"; "Iacute"; "Icirc";
    "Iuml"; "ETH"; "Ntilde"; "Ograve"; "Oacute"; "Ocirc"; "Otilde";
    "Ouml"; "times"; "Oslash"; "Ugrave"; "Uacute"; "Ucirc"; "Uuml";
    "Yacute"; "THORN"; "szlig"; "agrave"; "aacute"; "acirc";
    "atilde"; "auml"; "aring"; "aelig"; "ccedil"; "egrave";
    "eacute"; "ecirc"; "euml"; "igrave"; "iacute"; "icirc";
    "iuml"; "eth"; "ntilde"; "ograve"; "oacute"; "ocirc"; "otilde";
    "ouml"; "divide"; "oslash"; "ugrave"; "uacute"; "ucirc";
    "uuml"; "yacute"; "thorn"; "yuml"
  ] in
  let ent_uni_338_402 = [
    (* 338 (0x0152) *)
    "OElig"; "oelig"; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 352 (0x0160) *)
    "Scaron"; "scaron"; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 376 (0x0178) *)
    "Yuml"; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 400 (0x0190) *)
    ""; ""; "fnof"
  ] in
  let ent_uni_spacing = [
    (* 710 *)
    "circ";
    (* 711 - 730 *)
    ""; ""; ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; ""; ""; "";
    (* 731 - 732 *)
    ""; "tilde"
  ] in
  let ent_uni_greek = [
    (* 913 *)
    "Alpha"; "Beta"; "Gamma"; "Delta"; "Epsilon"; "Zeta"; "Eta"; "Theta";
    "Iota"; "Kappa"; "Lambda"; "Mu"; "Nu"; "Xi"; "Omicron"; "Pi"; "Rho";
    ""; "Sigma"; "Tau"; "Upsilon"; "Phi"; "Chi"; "Psi"; "Omega";
    (* 938 - 944 are not mapped *)
    ""; ""; ""; ""; ""; ""; "";
    "alpha"; "beta"; "gamma"; "delta"; "epsilon"; "zeta"; "eta"; "theta";
    "iota"; "kappa"; "lambda"; "mu"; "nu"; "xi"; "omicron"; "pi"; "rho";
    "sigmaf"; "sigma"; "tau"; "upsilon"; "phi"; "chi"; "psi"; "omega";
    (* 970 - 976 are not mapped *)
    ""; ""; ""; ""; ""; ""; "";
    "thetasym"; "upsih";
    ""; ""; "";
    "piv"
  ] in
  let ent_uni_punct = [
    (* 8194 *)
    "ensp"; "emsp"; ""; ""; ""; ""; "";
    "thinsp"; ""; ""; "zwnj"; "zwj"; "lrm"; "rlm";
    ""; ""; ""; "ndash"; "mdash"; ""; ""; "";
    (* 8216 *)
    "lsquo"; "rsquo"; "sbquo"; ""; "ldquo"; "rdquo"; "bdquo"; "";
    "dagger"; "Dagger"; "bull"; ""; ""; ""; "hellip";
    ""; ""; ""; ""; ""; ""; ""; ""; ""; "permil"; "";
    (* 8242 *)
    "prime"; "Prime"; ""; ""; ""; ""; ""; "lsaquo"; "rsaquo"; "";
    ""; ""; "oline"; ""; ""; ""; ""; "";
    "frasl"
  ] in
  let ent_uni_euro = [
    "euro"
  ] in
  let ent_uni_8465_8501 = [
    (* 8465 *)
    "image"; ""; ""; ""; ""; ""; "";
    (* 8472 *)
    "weierp"; ""; ""; "";
    (* 8476 *)
    "real"; ""; ""; ""; ""; "";
    (* 8482 *)
    "trade"; ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; ""; ""; "";
    (* 8501 *)
    "alefsym";
  ] in
  let ent_uni_8592_9002 = [
    (* 8592 (0x2190) *)
    "larr"; "uarr"; "rarr"; "darr"; "harr"; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8608 (0x21a0) *)
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8624 (0x21b0) *)
    ""; ""; ""; ""; ""; "crarr"; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8640 (0x21c0) *)
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8656 (0x21d0) *)
    "lArr"; "uArr"; "rArr"; "dArr"; "hArr"; "vArr"; ""; "";
    ""; ""; "lAarr"; "rAarr"; ""; "rarrw"; ""; "";
    (* 8672 (0x21e0) *)
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8704 (0x2200) *)
    "forall"; "comp"; "part"; "exist"; "nexist"; "empty"; ""; "nabla";
    "isin"; "notin"; "epsis"; "ni"; "notni"; "bepsi"; ""; "prod";
    (* 8720 (0x2210) *)
    "coprod"; "sum"; "minus"; "mnplus"; "plusdo"; ""; "setmn"; "lowast";
    "compfn"; ""; "radic"; ""; ""; "prop"; "infin"; "ang90";
    (* 8736 (0x2220) *)
    "ang"; "angmsd"; "angsph"; "mid"; "nmid"; "par"; "npar"; "and";
    "or"; "cap"; "cup"; "int"; ""; ""; "conint"; "";
    (* 8752 (0x2230) *)
    ""; ""; ""; ""; "there4"; "becaus"; ""; "";
    ""; ""; ""; ""; "sim"; "bsim"; ""; "";
    (* 8768 (0x2240) *)
    "wreath"; "nsim"; ""; "sime"; "nsime"; "cong"; ""; "ncong";
    "asymp"; "nap"; "ape"; ""; "bcong"; "asymp"; "bump"; "bumpe";
    (* 8784 (0x2250) *)
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8800 (0x2260) *)
    "ne"; "equiv"; ""; ""; "le"; "ge"; "lE"; "gE";
    "lnE"; "gnE"; "Lt"; "Gt"; "twixt"; ""; "nlt"; "ngt";
    (* 8816 (0x2270) *)
    "nles"; "nges"; "lsim"; "gsim"; ""; ""; "lg"; "gl";
    ""; ""; "pr"; "sc"; "cupre"; "sscue"; "prsim"; "scsim";
    (* 8832 (0x2280) *)
    "npr"; "nsc"; "sub"; "sup"; "nsub"; "nsup"; "sube"; "supe";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8848 (0x2290) *)
    ""; ""; ""; ""; ""; "oplus"; ""; "otimes";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8864 (0x22a0) *)
    ""; ""; ""; ""; ""; "perp"; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8880 (0x22b0) *)
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8896 (0x22c0) *)
    ""; ""; ""; ""; ""; "sdot"; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8912 (0x22d0) *)
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8928 (0x22e0) *)
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8944 (0x22f0) *)
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8960 (0x2300) *)
    ""; ""; ""; ""; ""; ""; ""; "";
    "lceil"; "rceil"; "lfloor"; "rfloor"; ""; ""; ""; "";
    (* 8976 (0x2310) *)
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; ""; ""; ""; ""; ""; ""; "";
    (* 8992 (0x2320) *)
    ""; ""; ""; ""; ""; ""; ""; "";
    ""; "lang"; "rang"
  ] in
  let ent_uni_9674 = [
    (* 9674 *)
    "loz"
  ] in
  let ent_uni_9824_9830 = [
    (* 9824 *)
    "spades"; ""; ""; "clubs"; ""; "hearts"; "diams"
  ] in
  let utf_entity_maps = [
    { start_char = 0xa0; table = ent_iso_8859_1 };
    { start_char = 338;  table = ent_uni_338_402 };
    { start_char = 710;  table = ent_uni_spacing };
    { start_char = 913;  table = ent_uni_greek };
    { start_char = 8194; table = ent_uni_punct };
    { start_char = 8364; table = ent_uni_euro };
    { start_char = 8465; table = ent_uni_8465_8501 };
    { start_char = 8592; table = ent_uni_8592_9002 };
    { start_char = 9674; table = ent_uni_9674 };
    { start_char = 9824; table = ent_uni_9824_9830 };
  ] in
  let decode_table = Caml.Hashtbl.create 0 in
  List.iter utf_entity_maps ~f: begin fun { start_char; table } ->
    List.iteri table ~f: begin fun i entity ->
      if String.length entity <> 0 then
        Caml.Hashtbl.add decode_table entity (utf32_to_utf8 (start_char + i))
    end
  end;
  let predefined = [
    "quot", "\"";
    "lt", "<";
    "gt", ">";
    "amp", "&";
    "apos", "\'";
    "cloud", utf32_to_utf8 0x2601;
    "umbrella", utf32_to_utf8 0x2602;
    "snowman", utf32_to_utf8 0x2603;
    "snowflake", utf32_to_utf8 0x2745;
    "comet", utf32_to_utf8 0x2604;
    "thunderstorm", utf32_to_utf8 0x2608
  ] in
  List.iter predefined ~f:(fun (k, v) -> Caml.Hashtbl.add decode_table k v);
  decode_table

let decode_entity s =
  (* check if entity has shape &#...
     - if yes - this is value of utf32 codepoint
       that should be converted to utf8 *)
  if String.get s 1 = '#'
  then begin
    assert (String.length s >= 3);
    try
      "0" ^ String.sub s 2 (String.length s - 3)
      |> int_of_string
      |> utf32_to_utf8
    with _ ->
      (* malformed entity - return empty string *)
      ""
  end
  else begin
    assert (String.length s >= 2);
    (* strip & and ; from the value and lookup result in decode table *)
    String.sub s 1 (String.length s - 2)
    |> Caml.Hashtbl.find_opt decode_table
    |> Option.value ~default:s
  end

let decode s = Str.global_substitute entity_regex (fun m ->
  decode_entity (Str.matched_string m)
) s
