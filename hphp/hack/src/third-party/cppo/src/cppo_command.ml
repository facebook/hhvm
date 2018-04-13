open Printf

type command_token =
    [ `Text of string
    | `Loc_file
    | `Loc_first_line
    | `Loc_last_line ]

type command_template = command_token list

let parse s : command_template =
  let rec loop acc buf s len i =
    if i >= len then
      let s = Buffer.contents buf in
      if s = "" then acc
      else `Text s :: acc
    else if i = len - 1 then (
      Buffer.add_char buf s.[i];
      `Text (Buffer.contents buf) :: acc
    )
    else
      let c = s.[i] in
      if c = '%' then
        let acc =
          let s = Buffer.contents buf in
          Buffer.clear buf;
          if s = "" then acc
          else
            `Text s :: acc
        in
        let x =
          match s.[i+1] with
              'F' -> `Loc_file
            | 'B' -> `Loc_first_line
            | 'E' -> `Loc_last_line
            | '%' -> `Text "%"
            | _ ->
                failwith (
                  sprintf "Invalid escape sequence in command template %S. \
                             Use %%%% for a %% sign." s
                )
        in
        loop (x :: acc) buf s len (i + 2)
      else (
        Buffer.add_char buf c;
        loop acc buf s len (i + 1)
      )
  in
  let len = String.length s in
  List.rev (loop [] (Buffer.create len) s len 0)


let subst (cmd : command_template) file first last =
  let l =
    List.map (
      function
          `Text s -> s
        | `Loc_file -> file
        | `Loc_first_line -> string_of_int first
        | `Loc_last_line -> string_of_int last
    ) cmd
  in
  String.concat "" l
