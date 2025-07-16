<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const string PHP_EOL;

const string PHP_OS;

const string PHP_VERSION;
const int PHP_MAJOR_VERSION;
const int PHP_MINOR_VERSION;
const int PHP_RELEASE_VERSION;
const int PHP_VERSION_ID;
const string PHP_EXTRA_VERSION;

const string HHVM_VERSION;
const string HHVM_COMPILER_ID;
const int HHVM_COMPILER_TIMESTAMP; // typechecker placeholder, see runtime
const string HHVM_REPO_SCHEMA;
const int HHVM_VERSION_ID;
const int HHVM_VERSION_MAJOR;
const int HHVM_VERSION_MINOR;
const int HHVM_VERSION_PATCH;

const int PHP_INT_MAX;
const int PHP_INT_MIN;
const int PHP_INT_SIZE;

const int DEBUG_BACKTRACE_PROVIDE_OBJECT;
const int DEBUG_BACKTRACE_IGNORE_ARGS;
const int DEBUG_BACKTRACE_PROVIDE_METADATA;
const int DEBUG_BACKTRACE_ONLY_METADATA_FRAMES;

const int E_ERROR;
const int E_WARNING;
const int E_PARSE;
const int E_NOTICE;
const int E_CORE_ERROR;
const int E_CORE_WARNING;
const int E_COMPILE_ERROR;
const int E_COMPILE_WARNING;
const int E_USER_ERROR;
const int E_USER_WARNING;
const int E_USER_NOTICE;
const int E_RECOVERABLE_ERROR;
const int E_DEPRECATED;
const int E_USER_DEPRECATED;
const int E_ALL;
const int E_STRICT;

// Built in pseudoconstants
newtype BuiltinPseudoConstantLineNumber as int = int;
const BuiltinPseudoConstantLineNumber __LINE__;

newtype BuiltinPseudoConstantName as string = string;

newtype BuiltinPseudoConstantClass as BuiltinPseudoConstantName = BuiltinPseudoConstantName;
const BuiltinPseudoConstantClass __CLASS__;

newtype BuiltinPseudoConstantTrait as BuiltinPseudoConstantName = BuiltinPseudoConstantName;
const BuiltinPseudoConstantTrait __TRAIT__;

newtype BuiltinPseudoConstantFile as BuiltinPseudoConstantName = BuiltinPseudoConstantName;
const BuiltinPseudoConstantFile __FILE__;

newtype BuiltinPseudoConstantDir as BuiltinPseudoConstantName = BuiltinPseudoConstantName;
const BuiltinPseudoConstantDir __DIR__;

newtype BuiltinPseudoConstantFunction as BuiltinPseudoConstantName = BuiltinPseudoConstantName;
const BuiltinPseudoConstantFunction __FUNCTION__;

newtype BuiltinPseudoConstantMethod as BuiltinPseudoConstantName = BuiltinPseudoConstantName;
const BuiltinPseudoConstantMethod __METHOD__;

newtype BuiltinPseudoConstantNamespace as BuiltinPseudoConstantName = BuiltinPseudoConstantName;
const BuiltinPseudoConstantNamespace __NAMESPACE__;

newtype BuiltinPseudoConstantCompilerFrontend as BuiltinPseudoConstantName = BuiltinPseudoConstantName;
const BuiltinPseudoConstantCompilerFrontend __COMPILER_FRONTEND__;

const FunctionCredential __FUNCTION_CREDENTIAL__;
