open Hh_prelude

(* The configuration of the CAS module parametrizes the location
    the version of the underlying external `recli` tool used for upload
    and download of large file artifacts and directories. *)
type config = {
  (* The directory to use as the install directory for `recli` *)
  install_dir: Path.t;
  (* The `recli` `fbpkg` version, which could be a version number,
      such as 6, or a tag, such as STABLE or DEFAULT *)
  version: string;
}
[@@deriving show]

type digest = Cas_digest of string [@@deriving show, eq]

type cas_failure =
  | Process_failure of string
  | Parse_failure of string

type cas_upload_result = {
  dir: Path.t;
  stdout: string;
}

type cas_download_result = {
  digest: digest;
  dir: Path.t;
  stdout: string;
}

type cas_result = {
  digest: digest;
  dir: Path.t;
}
[@@deriving show]

let upload_directory ?(timeout = 500) ~config (dir : Path.t) :
    (cas_result, cas_failure) result =
  ignore config;
  ignore timeout;
  ignore dir;
  failwith "not implemented"

let download_directory
    ?(timeout = 500) ~config (dir : Path.t) ~(digest : digest) =
  ignore config;
  ignore timeout;
  ignore dir;
  ignore digest;
  failwith "not implemented"

let begin_upload_directory config ~(dir : Path.t) :
    (cas_result, cas_failure) result Future.t =
  ignore dir;
  ignore config;
  failwith "not implemented"

let begin_download_directory config ~(dir : Path.t) ~(digest : digest) :
    (cas_result, cas_failure) result Future.t =
  ignore config;
  ignore dir;
  ignore digest;
  failwith "not implemented"

let to_string = function
  | Cas_digest digest -> digest

let of_string digest = Cas_digest digest
