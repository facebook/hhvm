/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

/*

This is the script that parses the configuration specification. The format is
following:

```
Any comments until we hit the first section

# SectionName

  Optional section comment paragraph that should be indented 2 spaces

- Type ConfigName[ = DefaultValue], Owner[, feature1|feature2...]

  Optional config comment paragraph that should be indented 2 spaces

```

Supported Types can be found in ConfigType

The ConfigName always need to be prefixed with the `SectionName.`. This is
done to make it easier to search

If no default value is applied you need to implement a custom function that
returns the default value. You can look at php7-impl.cpp for an example.

If the Owner is unknown use `UNKNOWN` as the owner.

Possible features are:
- private, will make the config private so it can only be used as default value
  for other configs in the same section
- globaldata(options), if the config should be part of RepoGlobalData.
  options is a pipe separated list of options.
  - noload: Do not load it in RepoGlobalData::load
- unitcacheflag, if the config should be part of the unit cache hash key
- repooptionsflag(name[, systemlibdefault]), if the config can be set in
  hhvmconfig.hdf inside the repo that contains the hack code. Use systemlibdefault
  incase the normal default is not a constant value
- compileroption(name), if the config is something the compiler needs
- lookuppath(name), use if you need to read from a different name then the configs
  name
- nobind, doesn't call Config::Bind which means it can't be set using hdf or
  command line
- postprocess, if the config needs a PostProcess method to process the value
  after it been read from the config
- staticdefault(value), if you need to set the default of the static variable
  to something other than the default for that type

*/

use std::collections::HashMap;
use std::collections::HashSet;
use std::fs;
use std::path::PathBuf;

use convert_case::Case;
use convert_case::Casing;
use nom::branch::alt;
use nom::bytes::complete::escaped;
use nom::bytes::complete::tag;
use nom::bytes::complete::take_until;
use nom::character::complete::alphanumeric1;
use nom::character::complete::digit1;
use nom::character::complete::newline;
use nom::character::complete::none_of;
use nom::character::complete::not_line_ending;
use nom::character::complete::one_of;
use nom::character::complete::space0;
use nom::character::complete::space1;
use nom::combinator::all_consuming;
use nom::combinator::cut;
use nom::combinator::eof;
use nom::combinator::map;
use nom::combinator::opt;
use nom::combinator::recognize;
use nom::combinator::value;
use nom::error::context;
use nom::error::ContextError;
use nom::error::ParseError;
use nom::multi::count;
use nom::multi::many0;
use nom::multi::many1;
use nom::multi::many_till;
use nom::multi::separated_list1;
use nom::sequence::delimited;
use nom::sequence::preceded;
use nom::sequence::tuple;
use nom::IResult;

#[derive(Debug, PartialEq, Clone)]
pub enum ConfigValue {
    Const(String),
    Name(String),
}

impl ConfigValue {
    fn string(&self, section_names: &Vec<String>) -> String {
        match self {
            ConfigValue::Const(v) => v.to_owned(),
            ConfigValue::Name(v) => {
                for sn in section_names {
                    if v.starts_with(&(sn.to_owned() + ".")) {
                        return format!(
                            "Cfg::{}::{}",
                            sn.replace('.', ""),
                            v.strip_prefix(&(sn.clone() + ".")).unwrap()
                        );
                    }
                }
                panic!("Unknown prefix {}", v);
            }
        }
    }
}

#[derive(Debug, PartialEq, Clone, Default)]
pub enum GlobalDataLoad {
    #[default]
    Always,
    OnlyHHBBC,
    Never,
}

#[derive(Debug, Hash, Eq, PartialEq, Clone, Default)]
pub enum HackcFlagType {
    #[default]
    HHBC,
    Parser,
}

#[derive(Debug, PartialEq, Clone)]
pub enum ConfigFeature {
    Private,
    GlobalData(GlobalDataLoad),
    UnitCacheFlag,
    RepoOptionsFlag(String, Option<String>),
    CompilerOption(String),
    LookupPath(String),
    NoBind,
    PostProcess,
    StaticDefault(String),
    HackcFlag(HackcFlagType, Option<String>),
}

#[derive(Debug, PartialEq, Default)]
pub struct ConfigFeatureGlobalData {
    pub load: GlobalDataLoad,
}

#[derive(Debug, PartialEq, Default)]
pub struct ConfigFeatureRepoOptionFlag {
    pub prefix: String,
    pub default_value: Option<String>,
}

#[derive(Debug, PartialEq, Default)]
pub struct ConfigFeatureHackcFlag {
    pub type_: HackcFlagType,
    pub default_value: Option<String>,
}

#[derive(Debug, PartialEq, Default)]
pub struct ConfigFeatures {
    pub is_repo_specific: bool,
    pub is_request_level: bool,
    pub is_private: bool,
    pub global_data: Option<ConfigFeatureGlobalData>,
    pub is_unit_cache_flag: bool,
    pub repo_options_flag: Option<ConfigFeatureRepoOptionFlag>,
    pub compiler_option: Option<String>,
    pub lookup_path: Option<String>,
    pub has_no_bind: bool,
    pub has_post_process: bool,
    pub static_default: Option<String>,
    pub hackc_flag: Option<ConfigFeatureHackcFlag>,
}

#[derive(Debug, PartialEq)]
pub struct Config {
    type_: ConfigType,
    pub name: String,
    pub default_value: Option<ConfigValue>,
    pub owner: Option<String>,
    pub description: Option<String>,
    pub features: ConfigFeatures,
}

impl Config {
    fn shortname(&self, section_name: &str) -> String {
        self.name
            .strip_prefix(&(section_name.to_owned() + "."))
            .unwrap()
            .replace('.', "")
    }

    fn hdf_path(&self, section: &ConfigSection) -> String {
        if let Some(lookup_path) = &self.features.lookup_path {
            return lookup_path.clone();
        }
        match &section.prefix {
            Some(prefix) => format!(
                "{}{}",
                prefix,
                self.name
                    .strip_prefix(&(section.name.to_owned() + "."))
                    .unwrap()
            ),
            None => self.name.to_owned(),
        }
    }
}

pub struct Include {
    pub name: String,
    pub user_defined: bool,
}

impl Include {
    fn user(name: &str) -> Include {
        Include {
            name: name.to_owned(),
            user_defined: true,
        }
    }

    fn library(name: &str) -> Include {
        Include {
            name: name.to_owned(),
            user_defined: false,
        }
    }

    fn str(&self) -> String {
        if self.user_defined {
            format!("#include \"{}\"", self.name)
        } else {
            format!("#include <{}>", self.name)
        }
    }
}

#[derive(Debug, PartialEq, Clone)]
pub enum ConfigType {
    Bool,
    Double,
    Int8t,
    Int16t,
    Int32t,
    Int64t,
    UInt8t,
    UInt16t,
    UInt32t,
    UInt64t,
    StdString,
    StdVectorStdString,
    StdSetStdString,
    StdMapStdStringStdString,
    BoostFlatSetStdString,
    HphpFastStringSet,
}

impl ConfigType {
    fn str(&self) -> &str {
        match *self {
            ConfigType::Bool => "bool",
            ConfigType::Double => "double",
            ConfigType::Int8t => "int8_t",
            ConfigType::Int16t => "int16_t",
            ConfigType::Int32t => "int32_t",
            ConfigType::Int64t => "int64_t",
            ConfigType::UInt8t => "uint8_t",
            ConfigType::UInt16t => "uint16_t",
            ConfigType::UInt32t => "uint32_t",
            ConfigType::UInt64t => "uint64_t",
            ConfigType::StdString => "std::string",
            ConfigType::StdVectorStdString => "std::vector<std::string>",
            ConfigType::StdSetStdString => "std::set<std::string>",
            ConfigType::StdMapStdStringStdString => "std::map<std::string,std::string>",
            ConfigType::BoostFlatSetStdString => "boost::container::flat_set<std::string>",
            ConfigType::HphpFastStringSet => "hphp_fast_string_set",
        }
    }

    fn includes(&self) -> Vec<Include> {
        match *self {
            ConfigType::Bool | ConfigType::Double => vec![],
            ConfigType::Int8t
            | ConfigType::Int16t
            | ConfigType::Int32t
            | ConfigType::Int64t
            | ConfigType::UInt8t
            | ConfigType::UInt16t
            | ConfigType::UInt32t
            | ConfigType::UInt64t => vec![Include::library("cstdint")],
            ConfigType::StdString => vec![Include::library("string")],
            ConfigType::StdVectorStdString => {
                vec![Include::library("vector"), Include::library("string")]
            }
            ConfigType::StdSetStdString => {
                vec![Include::library("set"), Include::library("string")]
            }
            ConfigType::StdMapStdStringStdString => {
                vec![Include::library("map"), Include::library("string")]
            }
            ConfigType::BoostFlatSetStdString => {
                vec![
                    Include::library("boost/container/flat_set.hpp"),
                    Include::library("string"),
                ]
            }
            ConfigType::HphpFastStringSet => vec![Include::user("hphp/util/hash-set.h")],
        }
    }

    fn default(&self) -> &str {
        match *self {
            ConfigType::Bool => "false",
            ConfigType::Double => "0.0",
            ConfigType::Int8t
            | ConfigType::Int16t
            | ConfigType::Int32t
            | ConfigType::Int64t
            | ConfigType::UInt8t
            | ConfigType::UInt16t
            | ConfigType::UInt32t
            | ConfigType::UInt64t => "0",
            ConfigType::StdString => "\"\"",
            ConfigType::StdVectorStdString => "{}",
            ConfigType::StdSetStdString => "{}",
            ConfigType::StdMapStdStringStdString => "{}",
            ConfigType::BoostFlatSetStdString => "{}",
            ConfigType::HphpFastStringSet => "{}",
        }
    }

    fn repo_option(&self) -> &str {
        match *self {
            ConfigType::StdVectorStdString => "Cfg::StringVector",
            ConfigType::StdSetStdString => "Cfg::StringSet",
            ConfigType::StdMapStdStringStdString => "Cfg::StringStringMap",
            ConfigType::BoostFlatSetStdString => "Cfg::StringBoostFlatSet",
            _ => self.str(),
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct ConfigSection {
    pub name: String,
    pub prefix: Option<String>,
    pub description: Option<String>,
    pub configs: Vec<Config>,
}

impl ConfigSection {
    fn shortname(&self) -> String {
        self.name.replace('.', "")
    }
}

fn parse_type<'a, E: ParseError<&'a str>>(input: &'a str) -> IResult<&'a str, ConfigType, E> {
    preceded(
        space0,
        alt((
            value(ConfigType::Bool, tag("bool")),
            value(ConfigType::Int8t, tag("int8_t")),
            value(ConfigType::Int16t, tag("int16_t")),
            value(ConfigType::Int32t, tag("int32_t")),
            value(ConfigType::Int64t, tag("int64_t")),
            value(ConfigType::Int32t, tag("int")),
            value(ConfigType::Double, tag("double")),
            value(ConfigType::UInt8t, tag("uint8_t")),
            value(ConfigType::UInt16t, tag("uint16_t")),
            value(ConfigType::UInt32t, tag("uint32_t")),
            value(ConfigType::UInt64t, tag("uint64_t")),
            value(ConfigType::StdString, tag("std::string")),
            value(
                ConfigType::StdVectorStdString,
                tag("std::vector<std::string>"),
            ),
            value(ConfigType::StdSetStdString, tag("std::set<std::string>")),
            value(
                ConfigType::StdMapStdStringStdString,
                tag("std::map<std::string,std::string>"),
            ),
            value(
                ConfigType::BoostFlatSetStdString,
                tag("boost::container::flat_set<std::string>"),
            ),
            value(ConfigType::HphpFastStringSet, tag("hphp_fast_string_set")),
        )),
    )(input)
}

fn parse_name<'a, E: ParseError<&'a str> + ContextError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, &'a str, E> {
    context(
        "parse name",
        recognize(tuple((
            alphanumeric1,
            many0(tuple((tag("_"), alphanumeric1))),
        ))),
    )(input)
}

fn parse_name_with_dot<'a, E: ParseError<&'a str> + ContextError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, &'a str, E> {
    context(
        "parse name with dot",
        preceded(
            space0,
            recognize(tuple((parse_name, many1(preceded(tag("."), parse_name))))),
        ),
    )(input)
}

fn parse_name_with_optional_dot<'a, E: ParseError<&'a str> + ContextError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, &'a str, E> {
    context(
        "parse name with optional dot",
        preceded(
            space0,
            recognize(tuple((parse_name, many0(preceded(tag("."), parse_name))))),
        ),
    )(input)
}

fn parse_partial_name<'a, E: ParseError<&'a str> + ContextError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, &'a str, E> {
    context(
        "parse partial name",
        preceded(
            space0,
            recognize(tuple((parse_name, many0(alt((tag("."), parse_name)))))),
        ),
    )(input)
}

fn parse_string<'a, E: ParseError<&'a str>>(input: &'a str) -> IResult<&'a str, &'a str, E> {
    recognize(delimited(
        tag("\""),
        opt(escaped(none_of(r#"\""#), '\\', one_of(r#"\""#))),
        tag("\""),
    ))(input)
}

fn parse_num<'a, E: ParseError<&'a str>>(input: &'a str) -> IResult<&'a str, &'a str, E> {
    alt((
        recognize(delimited(
            tuple((space0, tag("("))),
            parse_num_expr,
            tuple((space0, tag(")"))),
        )),
        recognize(tuple((
            space0,
            opt(alt((tag("-"), tag("+")))),
            alt((
                tag("INT_MAX"),
                tag("INT64_MAX"),
                tag("UINT64_MAX"),
                recognize(tuple((
                    alt((
                        recognize(tuple((digit1, opt(preceded(tag("."), digit1))))),
                        recognize(preceded(tag("."), digit1)),
                    )),
                    opt(alt((tag("LL"), tag("ll"), tag("ULL"), tag("ull")))),
                ))),
            )),
        ))),
    ))(input)
}

fn parse_num_expr<'a, E: ParseError<&'a str>>(input: &'a str) -> IResult<&'a str, &'a str, E> {
    recognize(tuple((
        parse_num,
        opt(many0(preceded(
            tuple((
                space0,
                alt((tag("*"), tag("+"), tag("-"), tag("/"), tag("<<"), tag(">>"))),
            )),
            parse_num,
        ))),
    )))(input)
}

fn parse_bool<'a, E: ParseError<&'a str>>(input: &'a str) -> IResult<&'a str, &'a str, E> {
    recognize(alt((tag("true"), tag("false"))))(input)
}

fn parse_constant_value<'a, E: ParseError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, &'a str, E> {
    alt((tag("{}"), parse_string, parse_num_expr, parse_bool))(input)
}

fn parse_value<'a, E: ParseError<&'a str> + ContextError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, ConfigValue, E> {
    alt((
        map(parse_constant_value, |v| ConfigValue::Const(v.to_string())),
        map(parse_name_with_dot, |v: &str| {
            ConfigValue::Name(v.to_string())
        }),
    ))(input)
}

fn parse_default_value<'a, E: ParseError<&'a str> + ContextError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, Option<ConfigValue>, E> {
    context(
        "default value",
        opt(preceded(
            tuple((space0, tag("="), space1)),
            cut(parse_value),
        )),
    )(input)
}

fn parse_owner<'a, E: ParseError<&'a str>>(input: &'a str) -> IResult<&'a str, Option<&'a str>, E> {
    map(
        preceded(tuple((space0, tag(","), space0)), alphanumeric1),
        |s: &str| {
            if s == "UNKNOWN" {
                return None;
            }
            Some(s)
        },
    )(input)
}

fn parse_features<'a, E: ParseError<&'a str> + ContextError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, ConfigFeatures, E> {
    map(
        opt(preceded(
            tuple((space0, tag(","), space0)),
            separated_list1(
                tag("|"),
                alt((
                    value(ConfigFeature::Private, tag("private")),
                    map(
                        tuple((
                            tag("globaldata"),
                            opt(delimited(
                                tag("("),
                                alt((
                                    value(GlobalDataLoad::Never, tag("noload")),
                                    value(GlobalDataLoad::OnlyHHBBC, tag("onlyhhbbc")),
                                )),
                                tag(")"),
                            )),
                        )),
                        |(_, load)| {
                            ConfigFeature::GlobalData(load.unwrap_or(GlobalDataLoad::Always))
                        },
                    ),
                    value(ConfigFeature::UnitCacheFlag, tag("unitcacheflag")),
                    map(
                        delimited(
                            tag("repooptionsflag("),
                            tuple((
                                parse_name_with_optional_dot,
                                opt(preceded(
                                    tuple((space0, tag(","), space0)),
                                    parse_constant_value,
                                )),
                            )),
                            tag(")"),
                        ),
                        |(name, default_value)| {
                            ConfigFeature::RepoOptionsFlag(
                                name.to_string(),
                                default_value.map(|v| v.to_string()),
                            )
                        },
                    ),
                    value(ConfigFeature::NoBind, tag("nobind")),
                    map(
                        delimited(
                            tag("compileroption("),
                            parse_name_with_optional_dot,
                            tag(")"),
                        ),
                        |name| ConfigFeature::CompilerOption(name.to_string()),
                    ),
                    map(
                        delimited(tag("lookuppath("), parse_name_with_optional_dot, tag(")")),
                        |name| ConfigFeature::LookupPath(name.to_string()),
                    ),
                    value(ConfigFeature::PostProcess, tag("postprocess")),
                    map(
                        delimited(tag("staticdefault("), parse_constant_value, tag(")")),
                        |name| ConfigFeature::StaticDefault(name.to_string()),
                    ),
                    map(
                        delimited(
                            tag("hackc("),
                            tuple((
                                alt((
                                    value(HackcFlagType::HHBC, tag("hhbc")),
                                    value(HackcFlagType::Parser, tag("parser")),
                                )),
                                opt(preceded(
                                    tuple((space0, tag(","), space0)),
                                    parse_constant_value,
                                )),
                            )),
                            tag(")"),
                        ),
                        |(hackc_type, default_value)| {
                            ConfigFeature::HackcFlag(
                                hackc_type,
                                default_value.map(|v| v.to_string()),
                            )
                        },
                    ),
                )),
            ),
        )),
        |v| {
            let mut features = ConfigFeatures::default();
            for f in v.unwrap_or(vec![]).into_iter() {
                match f {
                    ConfigFeature::Private => features.is_private = true,
                    ConfigFeature::GlobalData(load) => {
                        features.global_data = Some(ConfigFeatureGlobalData { load })
                    }
                    ConfigFeature::UnitCacheFlag => features.is_unit_cache_flag = true,
                    ConfigFeature::RepoOptionsFlag(prefix, default_value) => {
                        features.repo_options_flag = Some(ConfigFeatureRepoOptionFlag {
                            prefix,
                            default_value,
                        })
                    }
                    ConfigFeature::CompilerOption(name) => features.compiler_option = Some(name),
                    ConfigFeature::LookupPath(name) => features.lookup_path = Some(name),
                    ConfigFeature::NoBind => features.has_no_bind = true,
                    ConfigFeature::PostProcess => features.has_post_process = true,
                    ConfigFeature::StaticDefault(name) => features.static_default = Some(name),
                    ConfigFeature::HackcFlag(type_, default_value) => {
                        features.hackc_flag = Some(ConfigFeatureHackcFlag {
                            type_,
                            default_value,
                        })
                    }
                }
            }
            features
        },
    )(input)
}

fn parse_description<'a, E: ParseError<&'a str> + ContextError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, Option<String>, E> {
    context(
        "description",
        alt((
            preceded(
                count(newline, 2),
                opt(map(
                    many1(delimited(tag("  "), not_line_ending, newline)),
                    |s: Vec<&str>| s.join("\n"),
                )),
            ),
            value(None, many0(newline)),
        )),
    )(input)
}

fn parse_config<'a, E: ParseError<&'a str> + ContextError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, Config, E> {
    context(
        "config",
        map(
            preceded(
                tuple((opt(newline), tag("-"), space1)),
                tuple((
                    parse_type,
                    parse_name_with_dot,
                    parse_default_value,
                    parse_owner,
                    parse_features,
                    parse_description,
                )),
            ),
            |(type_, name, default_value, owner, features, description)| Config {
                type_,
                name: name.to_string(),
                default_value,
                owner: owner.map(|s| s.to_string()),
                features,
                description,
            },
        ),
    )(input)
}

fn parse_section<'a, E: ParseError<&'a str> + ContextError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, ConfigSection, E> {
    context(
        "section",
        map(
            tuple((
                preceded(
                    tuple((opt(newline), tag("#"), space1)),
                    parse_name_with_optional_dot,
                ),
                opt(delimited(
                    tuple((space1, tag("("))),
                    parse_partial_name,
                    tag(")"),
                )),
                parse_description,
                many1(parse_config),
            )),
            |(name, prefix, description, configs)| ConfigSection {
                name: name.to_string(),
                prefix: prefix.map(|s| s.to_string()),
                description,
                configs,
            },
        ),
    )(input)
}

pub fn parse_option_doc<'a, E: ParseError<&'a str> + ContextError<&'a str>>(
    input: &'a str,
) -> IResult<&'a str, Vec<ConfigSection>, E> {
    all_consuming(preceded(
        take_until("#"),
        map(many_till(parse_section, eof), |(cs, _r)| cs),
    ))(input)
}

pub fn generate_defs(sections: Vec<ConfigSection>, output_dir: PathBuf) {
    let section_names = sections.iter().map(|s| s.name.clone()).collect::<Vec<_>>();

    for section in sections.iter() {
        let section_shortname = section.shortname();
        let lower_section_shortname = section_shortname.to_lowercase();

        // [section].h file
        let mut public_defs = vec![];
        let mut private_defs = vec![];
        let mut includes = vec![];
        for config in section.configs.iter() {
            let shortname = config.shortname(&section.name);
            if config.features.repo_options_flag.is_none() {
                for include in config.type_.includes() {
                    includes.push(include);
                }
                if config.features.is_private {
                    private_defs.push(format!("  static {} {};", config.type_.str(), shortname));
                } else {
                    public_defs.push(format!("  static {} {};", config.type_.str(), shortname));
                }
            }
        }

        let mut unique_includes = includes
            .into_iter()
            .map(|p| p.str())
            .collect::<HashSet<_>>()
            .into_iter()
            .collect::<Vec<_>>();
        unique_includes.sort();

        let h_content = format!(
            r#"#pragma once

{}

namespace HPHP::Cfg {{

struct {} {{

  friend struct {}Loader;

{}

private:
{}

}};

}} // namespace HPHP::Cfg
"#,
            unique_includes.join("\n"),
            section_shortname,
            section_shortname,
            public_defs.join("\n"),
            private_defs.join("\n"),
        );
        let mut h_output_file = output_dir.clone();
        h_output_file.push(format!("{}.h", lower_section_shortname));
        fs::write(h_output_file, h_content).unwrap();

        // [section].cpp file
        let mut defs = vec![];
        for config in section.configs.iter() {
            let shortname = config.shortname(&section.name);
            if config.features.repo_options_flag.is_none() {
                defs.push(format!(
                    "{} {}::{} = {};",
                    config.type_.str(),
                    section_shortname,
                    shortname,
                    if config.features.has_no_bind && config.default_value.is_some() {
                        config
                            .default_value
                            .as_ref()
                            .unwrap()
                            .string(&section_names)
                    } else if let Some(d) = &config.features.static_default {
                        d.to_string()
                    } else {
                        config.type_.default().to_string()
                    },
                ));
            }
        }

        let cpp_content = format!(
            r#"#include "{}.h"

namespace HPHP::Cfg {{

{}

}} // namespace HPHP::Cfg
"#,
            lower_section_shortname,
            defs.join("\n"),
        );
        let mut cpp_output_file = output_dir.clone();
        cpp_output_file.push(format!("{}.cpp", lower_section_shortname));
        fs::write(cpp_output_file, cpp_content).unwrap();
    }
}

pub fn generate_hackc(sections: Vec<ConfigSection>, output_dir: PathBuf) {
    let mut hackc_flags_headers = HashMap::<HackcFlagType, Vec<String>>::new();
    hackc_flags_headers.insert(HackcFlagType::HHBC, vec![]);
    hackc_flags_headers.insert(HackcFlagType::Parser, vec![]);
    let mut hackc_flags_rs_properties = HashMap::<HackcFlagType, Vec<String>>::new();
    hackc_flags_rs_properties.insert(HackcFlagType::HHBC, vec![]);
    hackc_flags_rs_properties.insert(HackcFlagType::Parser, vec![]);
    let mut hackc_flags_rs_defaults = HashMap::<HackcFlagType, Vec<String>>::new();
    hackc_flags_rs_defaults.insert(HackcFlagType::HHBC, vec![]);
    hackc_flags_rs_defaults.insert(HackcFlagType::Parser, vec![]);
    let mut hackc_flags_rs_from_config = HashMap::<HackcFlagType, Vec<String>>::new();
    hackc_flags_rs_from_config.insert(HackcFlagType::HHBC, vec![]);
    hackc_flags_rs_from_config.insert(HackcFlagType::Parser, vec![]);
    let mut hackc_parser_to_parser_options = vec![];

    for section in sections.iter() {
        for config in section.configs.iter() {
            if let Some(hackc_flag) = &config.features.hackc_flag {
                let shortname = config.shortname(&section.name);
                let snake_case = shortname.from_case(Case::Camel).to_case(Case::Snake);

                hackc_flags_headers
                    .get_mut(&hackc_flag.type_)
                    .unwrap()
                    .push(format!("  {} {};", &config.type_.str(), snake_case));

                hackc_flags_rs_properties
                    .get_mut(&hackc_flag.type_)
                    .unwrap()
                    .push(format!("    pub {}: {},", snake_case, &config.type_.str()));

                hackc_flags_rs_defaults
                    .get_mut(&hackc_flag.type_)
                    .unwrap()
                    .push(format!(
                        "            {}: {},",
                        snake_case,
                        hackc_flag.default_value.as_ref().unwrap_or_else(|| {
                            if let Some(ConfigValue::Const(s)) = &config.default_value {
                                return s;
                            }
                            panic!("hackc need a constant default value");
                        })
                    ));

                hackc_flags_rs_from_config
                    .get_mut(&hackc_flag.type_)
                    .unwrap()
                    .push(format!(
                        r#"        if let Some(v) = config.get_bool("{}")? {{
                flags.{} = v;
            }}"#,
                        config.hdf_path(section),
                        snake_case,
                    ));

                if hackc_flag.type_ == HackcFlagType::Parser {
                    hackc_parser_to_parser_options
                        .push(format!("            {}: self.{},", snake_case, snake_case));
                }
            }
        }
    }

    // options_gen.h
    let hackc_options_generated_header_content = format!(
        r#"#pragma once

namespace HPHP::hackc {{

struct HhbcFlags {{
{}
}};

struct ParserFlags {{
{}
}};

}}"#,
        hackc_flags_headers[&HackcFlagType::HHBC].join("\n"),
        hackc_flags_headers[&HackcFlagType::Parser].join("\n"),
    );
    let mut hackc_options_generated_header_file = output_dir.clone();
    hackc_options_generated_header_file.push("options_gen.h");
    fs::write(
        hackc_options_generated_header_file,
        hackc_options_generated_header_content,
    )
    .unwrap();

    // options_gen.rs
    let hackc_options_generated_rs_content = format!(
        r#"use anyhow::Result;
use hhvm_options::HhvmConfig;
use oxidized::parser_options::ParserOptions;
use serde::Deserialize;
use serde::Serialize;

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize, Copy)]
#[repr(C)]
pub struct HhbcFlags {{
{}
}}

unsafe impl cxx::ExternType for HhbcFlags {{
    type Id = cxx::type_id!("HPHP::hackc::HhbcFlags");
    type Kind = cxx::kind::Trivial;
}}

impl Default for HhbcFlags {{
    fn default() -> Self {{
        Self {{
{}
        }}
    }}
}}

impl HhbcFlags {{
    pub fn from_config(config: &HhvmConfig) -> Result<Self> {{
        let mut flags = Self::default();
{}
        Ok(flags)
    }}
}}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize, Copy)]
#[repr(C)]
pub struct ParserFlags {{
{}
}}

unsafe impl cxx::ExternType for ParserFlags {{
    type Id = cxx::type_id!("HPHP::hackc::ParserFlags");
    type Kind = cxx::kind::Trivial;
}}

impl Default for ParserFlags {{
    fn default() -> Self {{
        Self {{
{}
        }}
    }}
}}

impl ParserFlags {{
    pub fn from_config(config: &HhvmConfig) -> Result<Self> {{
        let mut flags = Self::default();
{}
        Ok(flags)
    }}

    pub fn to_parser_options(&self) -> ParserOptions {{
        ParserOptions {{
{}
            ..Default::default()
        }}
    }}
}}"#,
        hackc_flags_rs_properties[&HackcFlagType::HHBC].join("\n"),
        hackc_flags_rs_defaults[&HackcFlagType::HHBC].join("\n"),
        hackc_flags_rs_from_config[&HackcFlagType::HHBC].join("\n"),
        hackc_flags_rs_properties[&HackcFlagType::Parser].join("\n"),
        hackc_flags_rs_defaults[&HackcFlagType::Parser].join("\n"),
        hackc_flags_rs_from_config[&HackcFlagType::Parser].join("\n"),
        hackc_parser_to_parser_options.join("\n"),
    );
    let mut hackc_options_generated_rs_file = output_dir.clone();
    hackc_options_generated_rs_file.push("options_gen.rs");
    fs::write(
        hackc_options_generated_rs_file,
        hackc_options_generated_rs_content,
    )
    .unwrap();
}

pub fn generate_loader(sections: Vec<ConfigSection>, output_dir: PathBuf) {
    let mut repo_global_data = vec![];
    let mut unit_cache_flags = vec![];
    let mut repo_options_includes = vec![];
    let mut repo_options_flags = vec![];
    let mut repo_options_sections = vec![];

    let section_names = sections.iter().map(|s| s.name.clone()).collect::<Vec<_>>();

    for section in sections.iter() {
        let section_shortname = section.shortname();
        let lower_section_shortname = section_shortname.to_lowercase();

        let has_hackc_hhbc_flags = section.configs.iter().any(|c| {
            if let Some(hackc_flag) = &c.features.hackc_flag {
                return hackc_flag.type_ == HackcFlagType::HHBC;
            }
            false
        });
        let has_hackc_parser_flags = section.configs.iter().any(|c| {
            if let Some(hackc_flag) = &c.features.hackc_flag {
                return hackc_flag.type_ == HackcFlagType::Parser;
            }
            false
        });

        let has_repo_option_flags = section
            .configs
            .iter()
            .any(|c| c.features.repo_options_flag.is_some());

        if has_repo_option_flags {
            repo_options_includes.push(format!(
                r#"#include "hphp/runtime/base/configs/{}-loader.h""#,
                lower_section_shortname
            ));
            repo_options_sections.push(format!("  S({}Loader)\\", section_shortname));
        }

        // [section]-loader.h
        let mut public_methods = vec![];
        let mut private_methods = vec![];
        for config in section.configs.iter() {
            let shortname = config.shortname(&section.name);
            if config.default_value.is_none() {
                public_methods.push(format!(
                    "  static {} {}Default();",
                    config.type_.str(),
                    shortname
                ));
            }
            if config.features.has_post_process {
                private_methods.push(format!(
                    "  static void {}PostProcess({}& value);",
                    shortname,
                    config.type_.str(),
                ));
            }
        }

        public_methods
            .push("  static void Load(const IniSettingMap& ini, const Hdf& config);".to_string());
        public_methods.push("  static std::string Debug();".to_string());
        if has_repo_option_flags {
            public_methods.push("  static void GetRepoOptionsFlags(RepoOptionsFlags& flags, const IniSettingMap& ini, const Hdf& config);".to_string());
            public_methods.push("  static void GetRepoOptionsFlagsFromConfig(RepoOptionsFlags& flags, const Hdf& config, const RepoOptionsFlags& default_flags);".to_string());
            public_methods.push(
                "  static void GetRepoOptionsFlagsForSystemlib(RepoOptionsFlags& flags);"
                    .to_string(),
            );
        }

        if has_hackc_hhbc_flags {
            public_methods.push("  static void InitHackcHHBCFlags(const RepoOptionsFlags& repo_flags, hackc::HhbcFlags& flags);".to_string());
        }
        if has_hackc_parser_flags {
            public_methods.push("  static void InitHackcParserFlags(const RepoOptionsFlags& repo_flags, hackc::ParserFlags& flags);".to_string());
        }

        let has_compiler_option = section
            .configs
            .iter()
            .any(|c| c.features.compiler_option.is_some());
        if has_compiler_option {
            public_methods.push(
                "  static void LoadForCompiler(const IniSettingMap& ini, const Hdf& config);"
                    .to_string(),
            );
        }

        let has_global_data_store = section
            .configs
            .iter()
            .any(|c| c.features.global_data.is_some());
        if has_global_data_store {
            public_methods.push("  static void StoreToGlobalData(RepoGlobalData& gd);".to_string());
        }

        let has_global_data_load = section.configs.iter().any(|c| {
            if let Some(gd) = &c.features.global_data {
                return gd.load == GlobalDataLoad::Always;
            }
            false
        });
        if has_global_data_load {
            public_methods
                .push("  static void LoadFromGlobalData(const RepoGlobalData& gd);".to_string());
        }

        let has_global_data_onlyhhbbc_load = section.configs.iter().any(|c| {
            if let Some(gd) = &c.features.global_data {
                return gd.load == GlobalDataLoad::OnlyHHBBC;
            }
            false
        });
        if has_global_data_onlyhhbbc_load {
            public_methods.push(
                "  static void LoadFromGlobalDataOnlyHHBBC(const RepoGlobalData& gd);".to_string(),
            );
        }

        let h_content = format!(
            r#"#pragma once

#include <string>

namespace HPHP {{

struct Hdf;
struct IniSettingMap;
struct RepoGlobalData;
struct RepoOptionsFlags;

namespace hackc {{
struct HhbcFlags;
struct ParserFlags;
}}

namespace Cfg {{

struct {}Loader {{

{}

private:
{}

}};

}} // namespace Cfg

}} // namespace HPHP

"#,
            section_shortname,
            public_methods.join("\n"),
            private_methods.join("\n"),
        );

        let mut h_output_file = output_dir.clone();
        h_output_file.push(format!("{}-loader.h", lower_section_shortname));
        fs::write(h_output_file, h_content).unwrap();

        // [section]-loader.cpp file
        let mut bind_calls = vec![];
        let mut debug_calls = vec![];
        let mut compiler_option_calls = vec![];
        let mut global_data_load_calls = vec![];
        let mut global_data_load_onlyhhbbc_calls = vec![];
        let mut global_data_store_calls = vec![];
        let mut repo_options_flags_calls = vec![];
        let mut repo_options_flags_from_config_calls = vec![];
        let mut repo_options_flags_for_systemlib_calls = vec![];
        let mut init_hhbc_flags_calls = vec![];
        let mut init_parser_flags_calls = vec![];
        for config in section.configs.iter() {
            let shortname = config.shortname(&section.name);
            let snake_shortname = shortname.from_case(Case::Camel).to_case(Case::Snake);
            let default_value = match &config.default_value {
                Some(v) => v.string(&section_names),
                None => format!("{}Default()", shortname),
            };
            if let Some(repo_option_flag) = &config.features.repo_options_flag {
                repo_options_flags_calls.push(format!(
                    r#"  Config::Bind(flags.{}, ini, config, "{}", {});"#,
                    shortname,
                    config.hdf_path(section),
                    default_value,
                ));
                repo_options_flags_from_config_calls.push(format!(
                    r#"  hdfExtract(config, "{}.{}", flags.{}, default_flags.{});"#,
                    repo_option_flag.prefix,
                    config.hdf_path(section),
                    shortname,
                    shortname
                ));
                repo_options_flags_for_systemlib_calls.push(format!(
                    r#"  flags.{} = {};"#,
                    shortname,
                    repo_option_flag.default_value.as_ref().unwrap_or_else(|| {
                        if let Some(ConfigValue::Const(s)) = &config.default_value {
                            return s;
                        }
                        panic!("repooptionflags need a constant default value");
                    })
                ));
                if let Some(hackc_flag) = &config.features.hackc_flag {
                    if hackc_flag.type_ == HackcFlagType::HHBC {
                        init_hhbc_flags_calls.push(format!(
                            r#"  flags.{} = repo_flags.{};"#,
                            snake_shortname, shortname,
                        ));
                    }
                    if hackc_flag.type_ == HackcFlagType::Parser {
                        init_parser_flags_calls.push(format!(
                            r#"  flags.{} = repo_flags.{};"#,
                            snake_shortname, shortname,
                        ));
                    }
                }
                debug_calls.push(format!(
                    r#"  fmt::format_to(std::back_inserter(out), "Cfg::{}::{} = Repo Option Flag so UNKNOWN!\n");"#,
                    section_shortname, shortname,
                ));
            } else {
                if !config.features.has_no_bind {
                    bind_calls.push(format!(
                        r#"  Config::Bind(Cfg::{}::{}, ini, config, "{}", {});"#,
                        section_shortname,
                        shortname,
                        config.hdf_path(section),
                        default_value
                    ));
                    if config.features.has_post_process {
                        bind_calls.push(format!(
                            r#"  {}PostProcess(Cfg::{}::{});"#,
                            shortname, section_shortname, shortname
                        ));
                    }
                } else if config.default_value.is_none() {
                    bind_calls.push(format!(
                        r#"  Cfg::{}::{} = {};"#,
                        section_shortname, shortname, default_value
                    ));
                }
                debug_calls.push(format!(
                    r#"  fmt::format_to(std::back_inserter(out), "Cfg::{}::{} = {{}}\n", Cfg::{}::{});"#,
                    section_shortname, shortname, section_shortname, shortname,
                ));
                if let Some(compiler_option_name) = &config.features.compiler_option {
                    compiler_option_calls.push(format!(
                        r#"  Config::Bind(Cfg::{}::{}, ini, config, "{}", Cfg::{}::{});"#,
                        section_shortname,
                        shortname,
                        compiler_option_name,
                        section_shortname,
                        shortname,
                    ));
                }

                if let Some(global_data) = &config.features.global_data {
                    match global_data.load {
                        GlobalDataLoad::Always => {
                            global_data_load_calls.push(format!(
                                r#"  Cfg::{}::{} = gd.{}_{};"#,
                                section_shortname, shortname, section_shortname, shortname
                            ));
                        }
                        GlobalDataLoad::OnlyHHBBC => {
                            global_data_load_onlyhhbbc_calls.push(format!(
                                r#"  Cfg::{}::{} = gd.{}_{};"#,
                                section_shortname, shortname, section_shortname, shortname
                            ));
                        }
                        GlobalDataLoad::Never => {}
                    }
                    global_data_store_calls.push(format!(
                        r#"  gd.{}_{} = Cfg::{}::{};"#,
                        section_shortname, shortname, section_shortname, shortname
                    ));
                }

                if let Some(hackc_flag) = &config.features.hackc_flag {
                    if hackc_flag.type_ == HackcFlagType::HHBC {
                        init_hhbc_flags_calls.push(format!(
                            r#"  flags.{} = Cfg::{}::{};"#,
                            snake_shortname, section_shortname, shortname,
                        ));
                    }
                    if hackc_flag.type_ == HackcFlagType::Parser {
                        init_parser_flags_calls.push(format!(
                            r#"  flags.{} = Cfg::{}::{};"#,
                            snake_shortname, section_shortname, shortname,
                        ));
                    }
                }
            }
        }

        let mut methods = vec![
            format!(
                r#"void {}Loader::Load(const IniSetting::Map& ini, const Hdf& config) {{
{}
}}"#,
                section_shortname,
                bind_calls.join("\n"),
            ),
            format!(
                r#"std::string {}Loader::Debug() {{
    std::string out;
{}
    return out;
}}"#,
                section_shortname,
                debug_calls.join("\n"),
            ),
        ];

        if has_repo_option_flags {
            methods.push(format!(
                r#"void {}Loader::GetRepoOptionsFlags(RepoOptionsFlags& flags, const IniSetting::Map& ini, const Hdf& config) {{
{}
}}

void {}Loader::GetRepoOptionsFlagsFromConfig(RepoOptionsFlags& flags, const Hdf& config, const RepoOptionsFlags& default_flags) {{
{}
}}

void {}Loader::GetRepoOptionsFlagsForSystemlib(RepoOptionsFlags& flags) {{
{}
}}"#,
                section_shortname,
                repo_options_flags_calls.join("\n"),
                section_shortname,
                repo_options_flags_from_config_calls.join("\n"),
                section_shortname,
                repo_options_flags_for_systemlib_calls.join("\n"),
                            ));
        }

        if has_hackc_hhbc_flags {
            methods.push(format!(
                                r#"void {}Loader::InitHackcHHBCFlags(const RepoOptionsFlags& repo_flags, hackc::HhbcFlags& flags) {{
{}
}}"#,
                                section_shortname,
                                init_hhbc_flags_calls.join("\n"),
                            ));
        }

        if has_hackc_parser_flags {
            methods.push(format!(
                                r#"void {}Loader::InitHackcParserFlags(const RepoOptionsFlags& repo_flags, hackc::ParserFlags& flags) {{
{}
}}"#,
                                section_shortname,
                                init_parser_flags_calls.join("\n"),
                            ));
        }

        if has_compiler_option {
            methods.push(format!(
                r#"void {}Loader::LoadForCompiler(const IniSettingMap& ini, const Hdf& config) {{
{}
}}"#,
                section_shortname,
                compiler_option_calls.join("\n"),
            ));
        }

        if has_global_data_load {
            methods.push(format!(
                r#"void {}Loader::LoadFromGlobalData(const RepoGlobalData& gd) {{
{}
}}"#,
                section_shortname,
                global_data_load_calls.join("\n"),
            ));
        }

        if has_global_data_onlyhhbbc_load {
            methods.push(format!(
                r#"void {}Loader::LoadFromGlobalDataOnlyHHBBC(const RepoGlobalData& gd) {{
{}
}}

"#,
                section_shortname,
                global_data_load_onlyhhbbc_calls.join("\n"),
            ));
        }

        if has_global_data_store {
            methods.push(format!(
                r#"void {}Loader::StoreToGlobalData(RepoGlobalData& gd) {{
{}
}}"#,
                section_shortname,
                global_data_store_calls.join("\n"),
            ));
        }

        let cpp_content = format!(
            r#"#include "hphp/runtime/base/configs/{}-loader.h"

#include "hphp/hack/src/hackc/compile/options_gen.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/util/configs/{}.h"
#include "hphp/util/hdf-extract.h"

#include <fmt/core.h>
#include <fmt/ranges.h>

namespace HPHP::Cfg {{

{}

}} // namespace HPHP::Cfg
"#,
            lower_section_shortname,
            lower_section_shortname,
            methods.join("\n\n"),
        );

        let mut cpp_output_file = output_dir.clone();
        cpp_output_file.push(format!("{}-loader.cpp", lower_section_shortname));
        fs::write(cpp_output_file, cpp_content).unwrap();

        // macro files
        for config in section.configs.iter() {
            let shortname = config.shortname(&section.name);
            if config.features.global_data.is_some() {
                repo_global_data.push(format!(
                    "  C(Cfg::{}::{}, {}_{}, {})\\",
                    section_shortname,
                    shortname,
                    section_shortname,
                    shortname,
                    &config.type_.str(),
                ));
            }
            if config.features.is_unit_cache_flag {
                unit_cache_flags.push(format!(
                    "  C(Cfg::{}::{}, {}_{}, {})\\",
                    section_shortname,
                    shortname,
                    section_shortname,
                    shortname,
                    &config.type_.str(),
                ));
            }
            if config.features.repo_options_flag.is_some() {
                repo_options_flags.push(format!(
                    "  C({}, {})\\",
                    &config.type_.repo_option(),
                    shortname
                ));
            }
        }
    }

    // repo-global-data-generated.h
    let repo_global_data_content = format!(
        r#"#pragma once
#define CONFIGS_FOR_REPOGLOBALDATA() \
{}

"#,
        repo_global_data.join("\n")
    );
    let mut repo_global_data_file = output_dir.clone();
    repo_global_data_file.push("repo-global-data-generated.h");
    fs::write(repo_global_data_file, repo_global_data_content).unwrap();

    // unit-cache-generated.h
    let unit_cache_content = format!(
        r#"#pragma once

#define CONFIGS_FOR_UNITCACHEFLAGS() \
{}

"#,
        unit_cache_flags.join("\n")
    );
    let mut unit_cache_file = output_dir.clone();
    unit_cache_file.push("unit-cache-generated.h");
    fs::write(unit_cache_file, unit_cache_content).unwrap();

    // repo-option-flags-generated.h
    let repo_options_flags_content = format!(
        r#"#pragma once

#include "hphp/runtime/base/configs/configs.h"
{}

#define CONFIGS_FOR_REPOOPTIONSFLAGS() \
{}

#define SECTIONS_FOR_REPOOPTIONSFLAGS() \
{}

"#,
        repo_options_includes.join("\n"),
        repo_options_flags.join("\n"),
        repo_options_sections.join("\n"),
    );
    let mut repo_option_flags_file = output_dir.clone();
    repo_option_flags_file.push("repo-options-flags-generated.h");
    fs::write(repo_option_flags_file, repo_options_flags_content).unwrap();

    let mut config_load_includes = vec![];
    let mut config_load_calls = vec![];
    let mut config_load_compiler_calls = vec![];
    let mut config_load_global_data_load_calls = vec![];
    let mut config_load_global_data_load_onlyhhbbc_calls = vec![];
    let mut config_load_global_data_store_calls = vec![];
    let mut repo_options_flags_calls = vec![];
    let mut repo_options_flags_from_config_calls = vec![];
    let mut repo_options_flags_for_systemlib_calls = vec![];
    let mut hackc_hhbc_flags_calls = vec![];
    let mut hackc_parser_flags_calls = vec![];

    for section in sections.iter() {
        let section_shortname = section.shortname();
        let lower_section_shortname = section_shortname.to_lowercase();
        config_load_includes.push(format!(
            r#"#include "hphp/runtime/base/configs/{}-loader.h""#,
            lower_section_shortname
        ));
        config_load_calls.push(format!(
            "  Cfg::{}Loader::Load(ini, config);",
            section_shortname
        ));
        if section
            .configs
            .iter()
            .any(|c| c.features.compiler_option.is_some())
        {
            config_load_compiler_calls.push(format!(
                "  Cfg::{}Loader::LoadForCompiler(ini, config);",
                section_shortname
            ));
        }

        if section
            .configs
            .iter()
            .any(|c| c.features.global_data.is_some())
        {
            config_load_global_data_store_calls.push(format!(
                "  Cfg::{}Loader::StoreToGlobalData(gd);",
                section_shortname
            ));
        }

        if section.configs.iter().any(|c| {
            if let Some(gd) = &c.features.global_data {
                return gd.load == GlobalDataLoad::Always;
            }
            false
        }) {
            config_load_global_data_load_calls.push(format!(
                "  Cfg::{}Loader::LoadFromGlobalData(gd);",
                section_shortname
            ));
        }

        if section.configs.iter().any(|c| {
            if let Some(gd) = &c.features.global_data {
                return gd.load == GlobalDataLoad::OnlyHHBBC;
            }
            false
        }) {
            config_load_global_data_load_onlyhhbbc_calls.push(format!(
                "  Cfg::{}Loader::LoadFromGlobalDataOnlyHHBBC(gd);",
                section_shortname
            ));
        }

        if section
            .configs
            .iter()
            .any(|c| c.features.repo_options_flag.is_some())
        {
            repo_options_flags_calls.push(format!(
                "  Cfg::{}Loader::GetRepoOptionsFlags(flags, ini, config);",
                section_shortname,
            ));
            repo_options_flags_from_config_calls.push(format!(
                "  Cfg::{}Loader::GetRepoOptionsFlagsFromConfig(flags, config, default_flags);",
                section_shortname,
            ));
            repo_options_flags_for_systemlib_calls.push(format!(
                "  Cfg::{}Loader::GetRepoOptionsFlagsForSystemlib(flags);",
                section_shortname,
            ));
        }

        if section.configs.iter().any(|c| {
            if let Some(hackc_flag) = &c.features.hackc_flag {
                return hackc_flag.type_ == HackcFlagType::HHBC;
            }
            false
        }) {
            hackc_hhbc_flags_calls.push(format!(
                "  Cfg::{}Loader::InitHackcHHBCFlags(repo_flags, flags);",
                section_shortname,
            ));
        }
        if section.configs.iter().any(|c| {
            if let Some(hackc_flag) = &c.features.hackc_flag {
                return hackc_flag.type_ == HackcFlagType::Parser;
            }
            false
        }) {
            hackc_parser_flags_calls.push(format!(
                "  Cfg::{}Loader::InitHackcParserFlags(repo_flags, flags);",
                section_shortname,
            ));
        }
    }

    let configs_load_content = format!(
        r#"#include "hphp/runtime/base/configs/configs.h"

{}

namespace HPHP {{

struct IniSettingMap;
struct Hdf;
struct RepoOptionsFlags;

namespace hackc {{
struct HhbcFlags;
struct ParserFlags;
}}

namespace Cfg {{

void Load(const IniSettingMap& ini, const Hdf& config) {{
{}
}}

void LoadForCompiler(const IniSettingMap& ini, const Hdf& config) {{
{}
}}

void StoreToGlobalData(RepoGlobalData& gd) {{
{}
}}

void LoadFromGlobalData(const RepoGlobalData& gd) {{
{}
}}

void LoadFromGlobalDataOnlyHHBBC(const RepoGlobalData& gd) {{
{}
}}

void GetRepoOptionsFlags(RepoOptionsFlags& flags, const IniSettingMap& ini, const Hdf& config) {{
{}
}}

void GetRepoOptionsFlagsFromConfig(RepoOptionsFlags& flags, const Hdf& config, const RepoOptionsFlags& default_flags) {{
{}
}}

void GetRepoOptionsFlagsForSystemlib(RepoOptionsFlags& flags) {{
{}
}}

void InitHackcHHBCFlags(const RepoOptionsFlags& repo_flags, hackc::HhbcFlags& flags) {{
{}
}}

void InitHackcParserFlags(const RepoOptionsFlags& repo_flags, hackc::ParserFlags& flags) {{
{}
}}

}} // namespace Cfg

}} // namespace HPHP
"#,
        config_load_includes.join("\n"),
        config_load_calls.join("\n"),
        config_load_compiler_calls.join("\n"),
        config_load_global_data_store_calls.join("\n"),
        config_load_global_data_load_calls.join("\n"),
        config_load_global_data_load_onlyhhbbc_calls.join("\n"),
        repo_options_flags_calls.join("\n"),
        repo_options_flags_from_config_calls.join("\n"),
        repo_options_flags_for_systemlib_calls.join("\n"),
        hackc_hhbc_flags_calls.join("\n"),
        hackc_parser_flags_calls.join("\n"),
    );
    let mut configs_load_file = output_dir.clone();
    configs_load_file.push("configs-generated.cpp");
    fs::write(configs_load_file, configs_load_content).unwrap();
}

#[cfg(test)]
mod test {
    use nom::error::VerboseError;

    use super::*;

    #[test]
    fn test_parse_string() {
        let res = parse_string::<VerboseError<&str>>(r#""foo""#);
        assert_eq!(res, Ok(("", r#""foo""#)));
        let res = parse_string::<VerboseError<&str>>(r#""f\\\"o\"o""#);
        assert_eq!(res, Ok(("", r#""f\\\"o\"o""#)));
    }

    #[test]
    fn test_parse_num_expr() {
        let res = parse_num_expr::<VerboseError<&str>>(r#"1 + 2 * 3 - 4 / 5 << 6"#);
        assert_eq!(res, Ok(("", r#"1 + 2 * 3 - 4 / 5 << 6"#)));
        let res = parse_num_expr::<VerboseError<&str>>(r#"( 1 + 2 )"#);
        assert_eq!(res, Ok(("", r#"( 1 + 2 )"#)));
        let res = parse_num_expr::<VerboseError<&str>>(r#"((25 << 10) + 1)"#);
        assert_eq!(res, Ok(("", r#"((25 << 10) + 1)"#)));
        let res = parse_num_expr::<VerboseError<&str>>(r#"(25 << 10) + 1"#);
        assert_eq!(res, Ok(("", r#"(25 << 10) + 1"#)));
        let res = parse_num_expr::<VerboseError<&str>>(r#"15 - (25 << 10) + 1"#);
        assert_eq!(res, Ok(("", r#"15 - (25 << 10) + 1"#)));
    }
}
