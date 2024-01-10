/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Defines the fields used in the query result structs

// don't show deprecation warnings about NewField when we build this file
#![allow(deprecated)]

use std::path::PathBuf;

use serde::Deserialize;
use serde_bser::bytestring::ByteString;

use crate::prelude::*;

/// This trait is used to furnish the caller with the watchman
/// field name for an entry in the file results
#[doc(hidden)]
pub trait QueryFieldName {
    fn field_name() -> &'static str;
}

/// This trait is used to produce the complete list of file
/// result field names for a query
#[doc(hidden)]
pub trait QueryFieldList {
    fn field_list() -> Vec<&'static str>;
}

/// This macro defines a field struct that can be composed using
/// the `query_result_type!` macro into a struct that can be used
/// with the `Client::query` method.
macro_rules! define_field {(
    $(#[$meta:meta])*
    $tyname:ident, $ty:ty, $field_name:literal) => {
        #[derive(Deserialize, Clone, Debug)]
        $(#[$meta])*
        pub struct $tyname {
            #[serde(rename = $field_name)]
            val: $ty,
        }

        impl QueryFieldName for $tyname {
            fn field_name() -> &'static str {
                $field_name
            }
        }

        impl $tyname {
            /// Fields should only be consumed when constructed via a QueryResult.
            /// This constructor is provided for users to easily mock out
            /// QueryResult objects in tests.
            pub fn new(val: $ty) -> Self {
                $tyname {
                    val,
                }
            }

            /// Consumes the field and returns the underlying
            /// value storage
            pub fn into_inner(self) -> $ty {
                self.val
            }
        }

        impl std::ops::Deref for $tyname {
            type Target = $ty;
            fn deref(&self) -> &Self::Target {
                &self.val
            }
        }

        impl std::ops::DerefMut for $tyname {
            fn deref_mut(&mut self) -> &mut Self::Target {
                &mut self.val
            }
        }
    };
}

define_field!(
    /// The field corresponding to the `name` of the file.
    NameField,
    PathBuf,
    "name"
);

define_field!(
    /// Like NameField but does not assume UTF-8.
    BytesNameField,
    ByteString,
    "name"
);

define_field!(
    /// The field corresponding to the `exists` status of the file
    ExistsField,
    bool,
    "exists"
);

define_field!(
    /// The field corresponding to the `cclock` field.
    /// the cclock is the created clock; the clock value when we first observed the file,
    /// or the clock value when it last switched from !exists to exists.
    CreatedClockField,
    ClockSpec,
    "cclock"
);

define_field!(
    /// The field corresponding to the `oclock` field.
    /// the oclock is the observed clock; the clock value where we last observed some
    /// change in this file or its metadata.
    ObservedClockField,
    ClockSpec,
    "oclock"
);

define_field!(
    /// The field corresponding to the `content.sha1hex` field.
    /// For regular files this evaluates to the sha1 hash of the
    /// file contents.
    ContentSha1HexField,
    ContentSha1Hex,
    "content.sha1hex"
);

define_field!(
    /// The field corresponding to the `ctime` field.
    /// ctime is the last inode change time measured in integer seconds since the
    /// unix epoch.
    CTimeField,
    i64,
    "ctime"
);

define_field!(
    /// The field corresponding to the `ctime_f` field.
    /// ctime is the last inode change time measured in floating point seconds
    /// (including the fractional portion) since the unix epoch.
    CTimeAsFloatField,
    f32,
    "ctime_f"
);

define_field!(
    /// The field corresponding to the `mtime` field.
    /// mtime is the last modified time measured in integer seconds
    /// since the unix epoch.
    MTimeField,
    i64,
    "mtime"
);

define_field!(
    /// The field corresponding to the `mtime_f` field.
    /// mtime is the last modified time measured in floating point seconds
    /// (including the fractional portion) since the unix epoch.
    MTimeAsFloatField,
    f32,
    "mtime_f"
);

define_field!(
    /// The field corresponding to the `size` field.
    /// This represents the size of the file in bytes.
    SizeField,
    u64,
    "size"
);

define_field!(
    /// The field corresponding to the `mode` field.
    /// This encodes the full file type and permission bits.
    /// Note that most programs and users are more comfortable with
    /// this value when printed in octal.
    /// It is recommended to use `FileTypeField` if all you need is the
    /// file type and not the permission bits, as it is cheaper to
    /// determine just the type in a virtualized filesystem.
    ModeAndPermissionsField,
    u64,
    "mode"
);

define_field!(
    /// The field corresponding to the `uid` field.
    /// The uid field is the owning uid expressed as an integer.
    /// This field is not meaningful on Windows.
    OwnerUidField,
    u32,
    "uid"
);

define_field!(
    /// The field corresponding to the `gid` field.
    /// The gid field is the owning gid expressed as an integer.
    /// This field is not meaningful on Windows.
    OwnerGidField,
    u32,
    "gid"
);

define_field!(
    /// The field corresponding to the `ino` field.
    /// The ino field is the inode number expressed as an integer.
    /// This field is not meaningful on Windows.
    InodeNumberField,
    u64,
    "ino"
);

define_field!(
    /// The field corresponding to the `dev` field.
    /// The dev field is the device number expressed as an integer.
    /// This field is not meaningful on Windows.
    DeviceNumberField,
    u64,
    "dev"
);

define_field!(
    /// The field corresponding to the `nlink` field.
    /// The nlink field is the number of hard links to the file
    /// expressed as an integer.
    NumberOfLinksField,
    u64,
    "nlink"
);

define_field!(
    /// The field corresponding to the `type` field.
    /// The type field encodes the type of the file.
    FileTypeField,
    FileType,
    "type"
);

define_field!(
    /// The field corresponding to the `symlink_target` field.
    /// For files of type symlink this evaluates to the result
    /// of readlink(2) on the file.
    SymlinkTargetField,
    Option<String>,
    "symlink_target"
);

define_field!(
    /// The field corresponding to the `new` field.
    /// The new field evaluates to true if a file is newer than
    /// the since generator criteria.
    ///
    /// Use of this field is discouraged as there are a number of
    /// situations in which the newness has a counter-intuitive
    /// value.  In addition, computing newness in a virtualized
    /// filesystem is relatively expensive.
    ///
    /// If your application needs to reason about the transition
    /// from `!exists -> exists` then you should track the
    /// `ExistsField` in your application.
    #[deprecated(note = "NewField can have counter-intuitive \
                         values in a number of situations so it \
                         is recommended that you track \
                         ExistsField instead")]
    NewField,
    bool,
    "new"
);

/// A macro to help define a type to hold file information from
/// a query.
/// This macro enables a type-safe way to define the set of fields
/// to be returned and de-serialize only those fields.
///
/// This defines a struct that will receive the name and content
/// hash fields from the results.  When used together with
/// `Client::query`, the query will automatically use the appropriate
/// list of field names:
///
/// ```
/// use serde::Deserialize;
/// use watchman_client::prelude::*;
///
/// query_result_type! {
///     struct NameAndHash {
///         name: NameField,
///         hash: ContentSha1HexField,
///     }
/// }
/// ```
///
/// The struct must consist of 2 or more fields; the macro subsystem
/// won't allow for generating an appropriate type definition for a single
/// field result.
///
/// If you need only a single field, look at [NameOnly](struct.NameOnly.html).
///
/// The field types must implement an undocumented trait that enables
/// the automatic field naming and correct deserialization regardless
/// of the field name in the struct.  As such, you should consider
/// the set of fields to be limited to those provided by this crate.
#[macro_export]
macro_rules! query_result_type {(
    $struct_vis:vis struct $tyname:ident {
        $($field_vis:vis $field_name:ident : $field_ty:ty),+ $(,)?
    }
    ) => (

#[derive(Deserialize, Debug, Clone)]
$struct_vis struct $tyname {
    $(
        #[serde(flatten)]
        $field_vis $field_name: $field_ty,
    )*
}

impl QueryFieldList for $tyname {
    fn field_list() -> Vec <&'static str> {
         vec![
        $(
            <$field_ty>::field_name(),
        )*
        ]
    }
}
    )
}

/// Use the `NameOnly` struct when your desired field list in your
/// query results consist only of the name field.
/// It is not possible to use the `query_result_type!` macro to define
/// an appropriate type due to limitations in the Rust macro system.
#[derive(Deserialize, Debug, Clone)]
#[serde(from = "PathBuf")]
pub struct NameOnly {
    pub name: NameField,
}

impl QueryFieldList for NameOnly {
    fn field_list() -> Vec<&'static str> {
        vec!["name"]
    }
}

impl From<PathBuf> for NameOnly {
    fn from(path: PathBuf) -> Self {
        Self {
            name: NameField { val: path },
        }
    }
}

#[derive(Deserialize, Debug, Clone)]
pub struct NoField {}

impl QueryFieldList for NoField {
    fn field_list() -> Vec<&'static str> {
        vec![]
    }
}
