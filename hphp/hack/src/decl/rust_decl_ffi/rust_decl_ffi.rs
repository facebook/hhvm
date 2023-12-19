// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::path::PathBuf;

use ast_and_decl_parser::Env;
use bumpalo::Bump;
use ocamlrep::bytes_from_ocamlrep;
use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep_caml_builtins::Int64;
use ocamlrep_custom::Custom;
use ocamlrep_ocamlpool::ocaml_ffi;
use ocamlrep_ocamlpool::ocaml_ffi_arena_result;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized_by_ref::direct_decl_parser::ParsedFile;
use oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes;
use parser_core_types::indexed_source_text::IndexedSourceText;
use relative_path::RelativePath;

#[derive(Debug, Clone)]
pub struct OcamlParsedFileWithHashes<'a>(ParsedFileWithHashes<'a>);

impl<'a> From<ParsedFileWithHashes<'a>> for OcamlParsedFileWithHashes<'a> {
    fn from(file: ParsedFileWithHashes<'a>) -> Self {
        Self(file)
    }
}

// NB: Must keep in sync with OCaml type `Direct_decl_parser.parsed_file_with_hashes`.
// Written manually because the underlying type doesn't implement ToOcamlRep;
// even if it did, its self.0.decls structure stores hh24_types::DeclHash
// but we here need an Int64. Writing manually is slicker than constructing
// a temporary vec.
impl ocamlrep::ToOcamlRep for OcamlParsedFileWithHashes<'_> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        let mut block = alloc.block_with_size(3);
        alloc.set_field(&mut block, 0, alloc.add(&self.0.mode));
        alloc.set_field(
            &mut block,
            1,
            alloc.add_copy(Int64(self.0.file_decls_hash.as_u64() as i64)),
        );
        let mut hd = alloc.add(&());
        for (name, decl, hash) in self.0.iter() {
            let mut tuple = alloc.block_with_size(3);
            alloc.set_field(&mut tuple, 0, alloc.add(name));
            alloc.set_field(&mut tuple, 1, alloc.add(decl));
            alloc.set_field(&mut tuple, 2, alloc.add_copy(Int64(hash.as_u64() as i64)));

            let mut cons_cell = alloc.block_with_size(2);
            alloc.set_field(&mut cons_cell, 0, tuple.build());
            alloc.set_field(&mut cons_cell, 1, hd);
            hd = cons_cell.build();
        }
        alloc.set_field(&mut block, 2, hd);
        block.build()
    }
}

ocaml_ffi_arena_result! {
    fn hh_parse_decls_ffi<'a>(
        arena: &'a Bump,
        opts: DeclParserOptions,
        filename: RelativePath,
        text: UnsafeOcamlPtr,
    ) -> ParsedFile<'a> {
        // SAFETY: Borrow the contents of the source file from the value on the
        // OCaml heap rather than copying it over. This is safe as long as we
        // don't call into OCaml within this function scope.
        let text_value: ocamlrep::Value<'a> = unsafe { text.as_value() };
        let text = bytes_from_ocamlrep(text_value).expect("expected string");
        direct_decl_parser::parse_decls_for_typechecking(&opts, filename, text, arena)
    }

    fn hh_parse_and_hash_decls_ffi<'a>(
        arena: &'a Bump,
        opts: DeclParserOptions,
        deregister_php_stdlib_if_hhi: bool,
        filename: RelativePath,
        text: UnsafeOcamlPtr,
    ) -> OcamlParsedFileWithHashes<'a> {
        let prefix = filename.prefix();
        // SAFETY: Borrow the contents of the source file from the value on the
        // OCaml heap rather than copying it over. This is safe as long as we
        // don't call into OCaml within this function scope.
        let text_value: ocamlrep::Value<'a> = unsafe { text.as_value() };
        let text = bytes_from_ocamlrep(text_value).expect("expected string");
        let parsed_file = direct_decl_parser::parse_decls_for_typechecking(&opts, filename, text, arena);
        let with_hashes = ParsedFileWithHashes::new(parsed_file, deregister_php_stdlib_if_hhi, prefix, arena);
        with_hashes.into()
    }
}

ocaml_ffi! {
    fn checksum_addremove_ffi(
        checksum: Int64,
        symbol: Int64,
        decl_hash: Int64,
        path: relative_path::RelativePath
    ) -> Int64 {
        // CARE! This implementation must be identical to the strongly-typed one in hh24_types.rs
        // I wrote it out as a separate copy because I didn't want hh_server to take a dependency
        // upon hh24_types
        let checksum = checksum.0 as u64;
        let checksum = checksum ^ hh_hash::hash(&(symbol, decl_hash, path));
        Int64(checksum as i64)
    }
}

#[no_mangle]
unsafe extern "C" fn hh_parse_ast_and_decls_ffi(env: usize, source_text: usize) -> usize {
    fn inner(env: usize, source_text: usize) -> usize {
        use ocamlrep::FromOcamlRep;
        use ocamlrep_ocamlpool::to_ocaml;
        use parser_core_types::source_text::SourceText;

        // SAFETY: We can't call into OCaml while these values created via
        // `from_ocaml` exist.
        let env = unsafe { Env::from_ocaml(env).unwrap() };
        let source_text = unsafe { SourceText::from_ocaml(source_text).unwrap() };
        let indexed_source_text = IndexedSourceText::new(source_text);

        let arena = &Bump::new();
        let (ast_result, decls) = ast_and_decl_parser::from_text(&env, &indexed_source_text, arena);
        // WARNING! this doesn't respect deregister_php_stdlib and is likely wrong.
        let decls = ParsedFileWithHashes::new_without_deregistering_do_not_use(decls);
        let decls = OcamlParsedFileWithHashes::from(decls);
        // SAFETY: Requires no concurrent interaction with the OCaml runtime
        unsafe { to_ocaml(&(ast_result, decls)) }
    }
    ocamlrep_ocamlpool::catch_unwind(|| inner(env, source_text))
}

extern "C" {
    fn caml_register_global_root(value: *mut UnsafeOcamlPtr);
    fn caml_remove_global_root(value: *mut UnsafeOcamlPtr);
}

/// "DeclsHolder" lets us allocate text+arena on a background thread, and move them
/// (and a reference to a decl allocated within arena with pointers to text) to the main thread.
/// We can't express lifetimes+ownerships properly short of orouboros.
/// So instead we lie: claim the parsed_file has 'static lifetime, even though
/// truly its lifetime is that of the arena+text.
/// This is also done in hackc/ffi_bridge/compiler_ffi.rs
///
/// SAFETY: no one must ever access `parsed_file` beyond the lifetime of
/// `arena` and `text`. (Rust doesn't let you mark entire struct members
/// as unsafe, so I'll make do with this comment.)
pub struct DeclsHolder {
    parsed_file: ParsedFile<'static>,
    #[allow(dead_code)]
    arena: bumpalo::Bump,
    #[allow(dead_code)]
    text: Vec<u8>,
}

struct SendableOcamlPtrPtr(*mut UnsafeOcamlPtr);

/// SAFETY:
/// Normally it's unsafe to move `*mut T` between threads. That's because Rust
/// has no way of knowing whether C will use thread-local-storage or thread-specific-locks
/// when using T. The safety requirement here is that you only send the pointer
/// from the main thread to a background thread, and then get it back onto the main
/// thread, before doing anything with it.
unsafe impl Send for SendableOcamlPtrPtr {}

/// "Concurrent" is used so a single-threaded ocaml consumer can use multiple concurrent rayon workers
/// to direct-decl-parse multiple files concurrently, and consume each result as soon as it's ready.
///
/// ## Single-threaded <-> multi-threaded interop:
/// The convenience, the interop bridge between single-threaded and multi-threaded, boils down to this function which is called
/// from the single-threaded ocaml side:
///   val enqueue_next_and_get_earlier_results : handle -> Relative_path.t list -> Relative_path.t * parsed_file) option
/// What is does is enqueue a list of workitems (relative-paths which we want direct-decl-parsed) onto the rust multithreaded side,
/// and then blocks synchronously until the next available result (a parsed_file) comes from rust. The next available result might
/// be for one of the workitems we placed just now. But it might also come from one of the earlier calls we made to this same function!
///
/// The implementation behind this is to use rayon:
/// For each relative-path, we do rayon::spawn to put the path (or rather, a lambda to process that path) into rayon's global workitem
/// queue. Rayon actually stores that queue amongst threads rather than having one single storage; it uses work stealing amongst threads.
/// Prior to spawn, we increment an integer outstanding. This integer lives on the main thread.
/// When the spawned lambda has finished direct-decl-parsing, it sends the results over a crossbeam channel.
/// The main thread will wait for the next available result over the crossbeam channel, and decrement the outstanding integer.
/// This way, if ever the integer is >0, we are guaranteed that a blocking wait on the channel will eventually be fulfilled.
///
/// ## Polymorphic tag, for metadata
/// I actually made the function take a polymorphic tag 'a for metadata which accompanies the workitem, and is sent back to ocaml
/// alongside that workitem's result, without ever having been read by the rust side.
///   val enqueue_next_and_get_earlier_results : handle -> (Relative_path.t * 'a) list -> (Relative_path.t * 'a * parsed_file) option
/// It's solely there as a handy place to store metadata. For instance, Decl_provider_prefetch.ml uses it because (1) knows
/// it wants to prefetch the ancestors for class C, (2) it enqueues a work item to direct-decl-parse c.php, (3) when it gets the
/// parse results back for the entire c.php, it needs to remember which class "C" it was prefetching ancestors for. The metadata
/// tag provides a place to store this.
pub struct Concurrent {
    /// This controls the behavior of `direct_decl_parser::parse_decls_for_typechecking`
    opts: DeclParserOptions,

    /// This provides repo-root and hhi-root, which are needed to turn a RelativePath into an abspath.
    /// We need this because workitems are given to us as RelativePath, and we on the multihtreaded
    /// rust side will read them from disk.
    ctx: relative_path::RelativePathCtx,

    /// This counts how many workitems have been kicked off, minus the ones whose results
    /// we have already received. It can naturally never be negative. If it is zero,
    /// then there must be no workitems pending.
    outstanding: usize,

    /// When a worker finishes parsing a file, it sends the results over channel `tx`.
    tx: crossbeam::channel::Sender<(RelativePath, SendableOcamlPtrPtr, DeclsHolder)>,

    /// When the main ocaml thread is waiting for a result from a worker, it receives
    /// that result over channel `rx`.
    rx: crossbeam::channel::Receiver<(RelativePath, SendableOcamlPtrPtr, DeclsHolder)>,
}

impl Concurrent {
    fn start(
        opts: DeclParserOptions,
        root: PathBuf,
        hhi: PathBuf,
        tmp: PathBuf,
        dummy: PathBuf,
    ) -> Self {
        let ctx = relative_path::RelativePathCtx {
            root,
            hhi,
            tmp,
            dummy,
        };
        // This is an unbounded channel. If rust workers go faster than the ocaml thread, then decls
        // will accumulate in rust memory inside the channel, until such time as they get copied over
        // to ocaml memory. That's fine.
        let (tx, rx) = crossbeam::channel::unbounded();
        Concurrent {
            opts,
            ctx,
            tx,
            rx,
            outstanding: 0,
        }
    }

    /// This function, although not marked as unsafe, really is unsafe: it must only ever
    /// be called from the ocaml thread. (please don't make it public!)
    fn step(
        &mut self,
        files: Vec<(RelativePath, UnsafeOcamlPtr, Option<String>)>,
    ) -> Option<(RelativePath, UnsafeOcamlPtr, UnsafeOcamlPtr)> {
        if self.outstanding == 0 {
            // This means that there are no workitems waiting to be processed.
            if files.is_empty() {
                // This means that the caller hasn't provided any additional
                // workitems. Therefore, no further results will ever come,
                // which we signal to our caller by returning None.
                return None;
            } else if files.len() == 1 {
                // A very common case is that our caller needs only one single
                // decl. I'm going to optimize that case, so it ends up having
                // identical implementation+performance to `hh_parse_decls_ffi` above,
                // with no concurrency.
                let (path, tag, content) = files.into_iter().next().unwrap();
                let abspath = path.to_absolute(&self.ctx);
                let text = match content {
                    Some(content) => content.into_bytes(),
                    None => std::fs::read(abspath).unwrap(),
                };
                let arena = bumpalo::Bump::new();
                let parsed_file = direct_decl_parser::parse_decls_for_typechecking(
                    &self.opts,
                    path.clone(),
                    &text,
                    &arena,
                );
                // SAFETY: ocamlrep_ocamlpool::to_ocaml must only ever be called from the main
                // ocaml thread. We fulfill that because we are only ever called from
                // an ocaml_ffi entrypoint.
                // (It would make sense to mark this entire function as unsafe, but I think it's
                // kind of nicer here to mark out which individual line inside it is unsafe).
                let parsed_file_ptr = unsafe { ocamlrep_ocamlpool::to_ocaml(&parsed_file) };
                // SAFETY: UnsafeOcamlPtr requires that the ocaml GC not run while the UnsafeOcamlPtr
                // exists. Well, we're returning it straight to ocaml right now, so it will no longer
                // exist at the end of this function before we get to ocaml land!
                return Some((path, tag, unsafe { UnsafeOcamlPtr::new(parsed_file_ptr) }));
            }
        }

        // If we failed to return early, that means *either* there are some existing workitems
        // which haven't yet been returned to ocaml, *or* that we've been given more than one
        // additional workitem.
        for (path, tag, content) in files {
            // The `tag` is an opaque ocaml reference. We want to store it alongside our workitem,
            // to be returned to ocaml once the workitem is done. The only way to achieve
            // this is to register it as an ocaml GC root: therefore, ocaml will be at liberty
            // to compact/move it, but we'll always be able to find it. The way this is implemented
            // is that we in rust allocate a box containing a pointer to the tag, and tell ocaml
            // the address of this box, and ocaml is at liberty to mutate the content of the box
            // as it moves the tag in memory.
            let tag = Box::new(tag);
            let tag = Box::into_raw(tag);
            // SAFETY: we should only ever register global roots from the main ocaml thread.
            // That's satisfied because our sole caller is an ocaml_ffi function.
            unsafe { caml_register_global_root(tag) };
            let tag = SendableOcamlPtrPtr(tag);
            let abspath = path.to_absolute(&self.ctx);
            let opts = self.opts.clone();
            let tx = self.tx.clone();
            self.outstanding += 1;
            // `outstanding` indicates that the main thread will know that, should it
            // block waiting for a result on `rx`, then that result will certainly
            // arrive. (hence, blocking on `rx` will not be a deadlock).
            // Our act of incrementing it here is a promise that we will send
            // exactly one result over `tx`.
            // We use `rayon::spawn`. This places the lambda into rayon's global
            // workitem queue, to be picked up by one of rayon's worker threads.
            rayon::spawn(move || {
                let text = match content {
                    Some(content) => content.into_bytes(),
                    None => std::fs::read(abspath).unwrap(),
                };
                let arena = bumpalo::Bump::new();
                // SAFETY: the effect of this transmute is that Rust no longer understands
                // that the lifetime of `parsed_file` is bounded by that of `text` and `arena`.
                // It will be up to us to enforce this ourselves.
                // We will return `parsed_file` in a `DeclsHolder` struct, the docblock
                // of which also mentions this safety constraint, so we'll leave it to
                // the consumer of that `DeclsHolder` to fulfill the constraint.
                let arena2 = unsafe {
                    std::mem::transmute::<&'_ bumpalo::Bump, &'static bumpalo::Bump>(&arena)
                };
                let text2 = unsafe { std::mem::transmute::<&'_ [u8], &'static [u8]>(&text) };
                let parsed_file: ParsedFile<'static> =
                    direct_decl_parser::parse_decls_for_typechecking(
                        &opts,
                        path.clone(),
                        text2,
                        arena2,
                    );
                let decls_holder = DeclsHolder {
                    parsed_file,
                    arena,
                    text,
                };
                let _ok_or_err = tx.send((path, tag, decls_holder));
            });
        }

        // Block, until the first available completed workitem is done -- either
        // from the ones we just kicked off just now, or one from an earlier
        // call to this function.
        let (path, SendableOcamlPtrPtr(tag), decls_holder) = self.rx.recv().unwrap();
        self.outstanding -= 1;

        // SAFETY: we must only ever remove global roots from the main ocaml thread.
        // That's satisfied because our sole caller is an ocaml_ffi.
        unsafe { caml_remove_global_root(tag) };

        // SAFETY: we require that "tag" have been allocated in a proper way, and that no one else has a reference
        // to it, since we now own the allocation and are free to drop it.
        // That's satisfied because (1) it was allocated by Box::new in the first place, (2) the only other
        // place that ever held a pointer was the ocaml root, and we removed that in the preceding statement.
        let tag = unsafe { Box::from_raw(tag) };

        // SAFETY: ocamlrep_ocamlpool::to_ocaml must only be called from the main ocaml thread.
        // That's satisfied, as before.
        let parsed_file_ptr = unsafe { ocamlrep_ocamlpool::to_ocaml(&decls_holder.parsed_file) };

        // SAFETY: decls_holder is an unsafe structure which lies about the lifetime
        // of `decls_holder.parsed_file`. No one must ever reference `parsed_file` after
        // `decls_holder.{text,arena}` have been dropped. Well, they don't! We've now
        // copied `parsed_file` over to the ocaml heap, and we'll never touch it again,
        // so it's safe to deallocate the arena+text.
        drop(decls_holder);

        // SAFETY: UnsafeOcamlPtr requires that the ocaml GC not run while it exists.
        // Well it won't exist once we return!
        Some((path, *tag, unsafe { UnsafeOcamlPtr::new(parsed_file_ptr) }))
    }
}

struct ConcurrentHandle(RefCell<Concurrent>);

impl ocamlrep_custom::CamlSerialize for ConcurrentHandle {
    // The "default_impls" support "drop" (when the ocaml GC collects then it will
    // drop the rust object), but doesn't suppoort "serialize" (when ocaml tries
    // to marshal or unmarshal then it will panic).
    ocamlrep_custom::caml_serialize_default_impls!();
}

ocaml_ffi! {
    fn hh_concurrent_parse_start_ffi(opts: DeclParserOptions, root: PathBuf, hhi: PathBuf, tmp: PathBuf, dummy: PathBuf) -> Custom<ConcurrentHandle> {
        Custom::from(ConcurrentHandle(RefCell::new(Concurrent::start(opts, root, hhi, tmp, dummy))))
    }

    fn hh_concurrent_parse_step_ffi(handle: Custom<ConcurrentHandle>, files: Vec<(RelativePath, UnsafeOcamlPtr, Option<String>)>) -> Option<(RelativePath, UnsafeOcamlPtr, UnsafeOcamlPtr)> {
        // This `borrow_mut` will never panic (i.e. the handle will never be borrowed at the time we call it).
        // That's because the only times we borrow are within the scope of ffi functions, and they always release,
        // and ocaml is single-threaded so there are never multiple ffi functions in flight at once.
        let mut handle = handle.0.borrow_mut();
        handle.step(files)
    }
}
