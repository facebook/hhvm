open Hh_prelude

type config = {
  install_dir: Path.t;
  version: string;
}
[@@deriving show]

type cas_failure =
  | Process_failure of string
  | Parse_failure of string

type cas_upload_result

type cas_download_result

type digest = Cas_digest of string [@@deriving show, eq]

type cas_result = {
  digest: digest;
  dir: Path.t;
}
[@@deriving show]

val begin_upload_directory :
  config -> dir:Path.t -> (cas_result, cas_failure) result Future.t

val upload_directory :
  config -> ?timeout:int -> dir:Path.t -> (cas_result, cas_failure) result

val begin_download_directory :
  config ->
  dir:Path.t ->
  digest:digest ->
  (cas_result, cas_failure) result Future.t

val download_directory :
  config ->
  ?timeout:int ->
  dir:Path.t ->
  digest:digest ->
  (cas_result, cas_failure) result

val of_string : string -> digest

val to_string : digest -> string
