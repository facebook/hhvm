// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocaml::{caml, Str, Value};

// Unix.file_descr -> sharedmem_base_address -> string -> int = "get_gconst"
// Send an RPC request over file-descriptor to the decl service,
// block until it sends back an offset, and (not yet implemented) return
// the native ocaml value located at sharedmem+offset.
// For now it does the RPC but returns a dummy ocaml value.
// The ocaml signature is
//   type addr  (** opaque pointer for base address of sharedmem *)
//   external get_gconst : Unix.file_descr -> addr -> string -> [dummy:int]
// The input string is the gconst symbol name we want to look up.
// TODO: figure out how we want to expose errors to ocaml...
// I'm eager that the common path should involve no allocations
caml!(get_gconst, |fd, base, name|, <result>, {
    let fd = fd.i32_val() as std::os::unix::io::RawFd;
    let _base = base.ptr_val::<*const u8>();
    let name = Str::from(name);
    let name = name.as_str();
    let kind = 1u32;
    decl_ipc::write_request(fd, &name, kind).unwrap();
    let _offset = decl_ipc::read_response(fd).unwrap();
    // TODO: once cachelib has real ocaml data in its blobs, return a Ptr to that.
    // Here's just a placeholder.
    result = Value::i32(271);
} -> result);
