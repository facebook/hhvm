// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;

use anyhow::Result;
use hh24_types::ToplevelSymbolHash;
use maplit::btreemap;
use oxidized::naming_types;
use pos::ConstName;
use pos::FunName;
use pos::Prefix;
use pos::RelativePath;
use pos::TypeName;
use rpds::HashTrieSet;

use super::NamingTable;

// stub for hh_shared.c function called by shm_store crate
#[no_mangle]
extern "C" fn hh_log_level() -> ocamlrep::Value<'static> {
    ocamlrep::Value::int(0)
}

fn setup(files: std::collections::BTreeMap<&str, &str>) -> (hh24_test::TestRepo, NamingTable) {
    let repo = hh24_test::TestRepo::new(&files).unwrap();
    let db_path = repo.path().join("names.sql");
    hh24_test::create_naming_table(&db_path, &files).unwrap();
    let naming_table = NamingTable::new(Some(db_path)).unwrap();
    shm_init();
    (repo, naming_table)
}

fn shm_init() {
    let mem_heap_size = 2 * 1024 * 1024 * 1024;
    let mmap_ptr = unsafe {
        libc::mmap(
            std::ptr::null_mut(),
            mem_heap_size,
            libc::PROT_READ | libc::PROT_WRITE,
            libc::MAP_SHARED | libc::MAP_ANONYMOUS,
            -1,
            0,
        )
    };
    assert_ne!(mmap_ptr, libc::MAP_FAILED);
    shmffi::shmffi_init(mmap_ptr, mem_heap_size, (mem_heap_size / 2) as isize);
}

fn make_dep_from_typename(typename: &str) -> deps_rust::Dep {
    deps_rust::Dep::new(ToplevelSymbolHash::from_type(typename).as_u64())
}

fn make_relative_path_from_str(test_path: &str) -> RelativePath {
    RelativePath::new(Prefix::Root, PathBuf::from(test_path))
}

#[test]
fn type_test() -> Result<()> {
    let (_repo, naming_table) = setup(btreemap! {
        "a.php" => "class A extends B {}",
        "b.php" => "class B {}"
    });

    let a_type = TypeName::new(r#"\A"#);
    let a_relative_path = make_relative_path_from_str("a.php");

    // Retrieve a typename
    let (pos, kindof) = naming_table.get_type_pos(a_type).unwrap().unwrap();

    let rp: RelativePath = pos.path();
    assert_eq!(rp, a_relative_path);
    assert_eq!(kindof, naming_types::KindOfType::TClass);

    // Remove a typename
    naming_table.remove_type_batch(&[a_type])?;
    assert_eq!(naming_table.get_type_pos(a_type).unwrap(), None);

    // Add a typename
    naming_table.add_type(
        a_type,
        &(
            oxidized::file_info::Pos::File(
                oxidized::file_info::NameType::Class,
                std::sync::Arc::new(a_relative_path.into()),
            ),
            naming_types::KindOfType::TClass,
        ),
    )?;

    let (pos, kindof) = naming_table.get_type_pos(a_type).unwrap().unwrap();
    let rp: RelativePath = pos.path();
    assert_eq!(rp, a_relative_path);
    assert_eq!(kindof, naming_types::KindOfType::TClass);

    // Get name from its lowercase version
    assert_eq!(
        naming_table
            .get_canon_type_name(TypeName::new(r#"\a"#))
            .unwrap()
            .unwrap(),
        a_type
    );

    Ok(())
}

#[test]
fn fun_test() -> Result<()> {
    let (_repo, naming_table) = setup(btreemap! {
        "a.php" => "function A(): void { b(); }",
        "b.php" => "function b(): void { A(); }",
    });

    let a_fun = FunName::new(r#"\A"#);
    let a_relative_path = make_relative_path_from_str("a.php");

    // Retrieve a fun
    let pos = naming_table.get_fun_pos(a_fun).unwrap().unwrap();
    let rp: RelativePath = pos.path();
    assert_eq!(rp, a_relative_path);

    // Remove a fun
    naming_table.remove_fun_batch(&[a_fun])?;
    assert_eq!(naming_table.get_fun_pos(a_fun).unwrap(), None);

    // Add a fun
    naming_table.add_fun(
        a_fun,
        &oxidized::file_info::Pos::File(
            oxidized::file_info::NameType::Fun,
            std::sync::Arc::new(a_relative_path.into()),
        ),
    )?;

    let pos = naming_table.get_fun_pos(a_fun).unwrap().unwrap();
    let rp: RelativePath = pos.path();
    assert_eq!(rp, a_relative_path);

    // Get canon name from its lowercase version
    assert_eq!(
        naming_table
            .get_canon_fun_name(FunName::new(r#"\a"#))
            .unwrap()
            .unwrap(),
        a_fun
    );

    Ok(())
}

#[test]
fn const_test() -> Result<()> {
    let (_repo, naming_table) = setup(btreemap! {
        "a.php" => "const int A = 123;",
        "lowercase_a.php" => "const int a = 321;",
    });

    let a_const = ConstName::new(r#"\A"#);
    let a_relative_path = make_relative_path_from_str("a.php");

    // Retrieve a const
    let pos = naming_table.get_const_pos(a_const).unwrap().unwrap();
    let rp: RelativePath = pos.path();
    assert_eq!(rp, a_relative_path);

    // Remove a const
    naming_table.remove_const_batch(&[a_const])?;
    assert_eq!(naming_table.get_const_pos(a_const).unwrap(), None);

    // Add a const
    naming_table.add_const(
        a_const,
        &oxidized::file_info::Pos::File(
            oxidized::file_info::NameType::Const,
            std::sync::Arc::new(a_relative_path.into()),
        ),
    )?;

    let pos = naming_table.get_const_pos(a_const).unwrap().unwrap();
    let rp: RelativePath = pos.path();
    assert_eq!(rp, a_relative_path);

    Ok(())
}

#[test]
fn get_filenames_by_hash_test() -> Result<()> {
    let (_repo, naming_table) = setup(btreemap! {
        "a.php" => "class A extends B {}",
        "b.php" => "class B {}"
    });

    let a_type = make_dep_from_typename(r#"\A"#);
    let b_type = make_dep_from_typename(r#"\B"#);
    let a_relative_path = make_relative_path_from_str("a.php");
    let b_relative_path = make_relative_path_from_str("b.php");

    let depset = HashTrieSet::new();
    let depset = depset.insert(a_type);
    let dep_paths = naming_table
        .get_filenames_by_hash(&deps_rust::DepSet::from(depset))
        .unwrap();
    assert!(dep_paths.contains(&a_relative_path));
    assert!(!dep_paths.contains(&b_relative_path));

    let depset = HashTrieSet::new();
    let depset = depset.insert(a_type);
    let depset = depset.insert(b_type);
    let dep_paths = naming_table
        .get_filenames_by_hash(&deps_rust::DepSet::from(depset))
        .unwrap();
    assert!(dep_paths.contains(&a_relative_path));
    assert!(dep_paths.contains(&b_relative_path));

    let depset = HashTrieSet::new();
    let dep_paths = naming_table
        .get_filenames_by_hash(&deps_rust::DepSet::from(depset))
        .unwrap();
    assert!(dep_paths.is_empty());

    let depset = HashTrieSet::new();
    let depset = depset.insert(make_dep_from_typename(r#"\C"#));
    let dep_paths = naming_table
        .get_filenames_by_hash(&deps_rust::DepSet::from(depset))
        .unwrap();
    assert!(dep_paths.is_empty());
    Ok(())
}
