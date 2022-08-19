/* Copyright (c) 2005, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

// Partial locale(7) support.

#include "sql/sql_locale.h"

#include <stddef.h>

#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#include "sql/derror.h"  // MY_LOCALE_ERRMSGS
#include "sql/log.h"
#include "sql/sql_error.h"
#include "typelib.h"

enum err_msgs_index {
  en_US = 0,
  cs_CZ,
  da_DK,
  nl_NL,
  et_EE,
  fr_FR,
  de_DE,
  el_GR,
  hu_HU,
  it_IT,
  ja_JP,
  ko_KR,
  no_NO,
  nn_NO,
  pl_PL,
  pt_PT,
  ro_RO,
  ru_RU,
  sr_RS,
  sk_SK,
  es_ES,
  sv_SE,
  uk_UA
} ERR_MSGS_INDEX;

MY_LOCALE_ERRMSGS global_errmsgs[] = {
    MY_LOCALE_ERRMSGS("english"),   MY_LOCALE_ERRMSGS("czech"),
    MY_LOCALE_ERRMSGS("danish"),    MY_LOCALE_ERRMSGS("dutch"),
    MY_LOCALE_ERRMSGS("estonian"),  MY_LOCALE_ERRMSGS("french"),
    MY_LOCALE_ERRMSGS("german"),    MY_LOCALE_ERRMSGS("greek"),
    MY_LOCALE_ERRMSGS("hungarian"), MY_LOCALE_ERRMSGS("italian"),
    MY_LOCALE_ERRMSGS("japanese"),  MY_LOCALE_ERRMSGS("korean"),
    MY_LOCALE_ERRMSGS("norwegian"), MY_LOCALE_ERRMSGS("norwegian-ny"),
    MY_LOCALE_ERRMSGS("polish"),    MY_LOCALE_ERRMSGS("portuguese"),
    MY_LOCALE_ERRMSGS("romanian"),  MY_LOCALE_ERRMSGS("russian"),
    MY_LOCALE_ERRMSGS("serbian"),   MY_LOCALE_ERRMSGS("slovak"),
    MY_LOCALE_ERRMSGS("spanish"),   MY_LOCALE_ERRMSGS("swedish"),
    MY_LOCALE_ERRMSGS("ukrainian"), MY_LOCALE_ERRMSGS(nullptr)};

/***** LOCALE BEGIN ar_AE: Arabic - United Arab Emirates *****/
static const char *my_locale_month_names_ar_AE[13] = {
    "يناير", "فبراير", "مارس",   "أبريل",  "مايو",   "يونيو", "يوليو",
    "أغسطس", "سبتمبر", "أكتوبر", "نوفمبر", "ديسمبر", NullS};
static const char *my_locale_ab_month_names_ar_AE[13] = {
    "ينا", "فبر", "مار", "أبر", "ماي", "يون", "يول",
    "أغس", "سبت", "أكت", "نوف", "ديس", NullS};
static const char *my_locale_day_names_ar_AE[8] = {
    "الاثنين", "الثلاثاء", "الأربعاء", "الخميس",
    "الجمعة",  "السبت ",   "الأحد",    NullS};
static const char *my_locale_ab_day_names_ar_AE[8] = {"ن", "ث", "ر", "خ",
                                                      "ج", "س", "ح", NullS};
static TYPELIB my_locale_typelib_month_names_ar_AE = {
    array_elements(my_locale_month_names_ar_AE) - 1, "",
    my_locale_month_names_ar_AE, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ar_AE = {
    array_elements(my_locale_ab_month_names_ar_AE) - 1, "",
    my_locale_ab_month_names_ar_AE, nullptr};
static TYPELIB my_locale_typelib_day_names_ar_AE = {
    array_elements(my_locale_day_names_ar_AE) - 1, "",
    my_locale_day_names_ar_AE, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ar_AE = {
    array_elements(my_locale_ab_day_names_ar_AE) - 1, "",
    my_locale_ab_day_names_ar_AE, nullptr};
MY_LOCALE my_locale_ar_AE(6, "ar_AE", "Arabic - United Arab Emirates", false,
                          &my_locale_typelib_month_names_ar_AE,
                          &my_locale_typelib_ab_month_names_ar_AE,
                          &my_locale_typelib_day_names_ar_AE,
                          &my_locale_typelib_ab_day_names_ar_AE, 6, 8,
                          '.',    /* decimal point ar_AE */
                          ',',    /* thousands_sep ar_AE */
                          "\x03", /* grouping      ar_AE */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_AE *****/

/***** LOCALE BEGIN ar_BH: Arabic - Bahrain *****/
static const char *my_locale_month_names_ar_BH[13] = {
    "يناير", "فبراير", "مارس",   "أبريل",  "مايو",   "يونيو", "يوليو",
    "أغسطس", "سبتمبر", "أكتوبر", "نوفمبر", "ديسمبر", NullS};
static const char *my_locale_ab_month_names_ar_BH[13] = {
    "ينا", "فبر", "مار", "أبر", "ماي", "يون", "يول",
    "أغس", "سبت", "أكت", "نوف", "ديس", NullS};
static const char *my_locale_day_names_ar_BH[8] = {
    "الاثنين", "الثلاثاء", "الأربعاء", "الخميس",
    "الجمعة",  "السبت",    "الأحد",    NullS};
static const char *my_locale_ab_day_names_ar_BH[8] = {"ن", "ث", "ر", "خ",
                                                      "ج", "س", "ح", NullS};
static TYPELIB my_locale_typelib_month_names_ar_BH = {
    array_elements(my_locale_month_names_ar_BH) - 1, "",
    my_locale_month_names_ar_BH, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ar_BH = {
    array_elements(my_locale_ab_month_names_ar_BH) - 1, "",
    my_locale_ab_month_names_ar_BH, nullptr};
static TYPELIB my_locale_typelib_day_names_ar_BH = {
    array_elements(my_locale_day_names_ar_BH) - 1, "",
    my_locale_day_names_ar_BH, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ar_BH = {
    array_elements(my_locale_ab_day_names_ar_BH) - 1, "",
    my_locale_ab_day_names_ar_BH, nullptr};
MY_LOCALE my_locale_ar_BH(7, "ar_BH", "Arabic - Bahrain", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_BH */
                          ',',    /* thousands_sep ar_BH */
                          "\x03", /* grouping      ar_BH */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_BH *****/

/***** LOCALE BEGIN ar_JO: Arabic - Jordan *****/
static const char *my_locale_month_names_ar_JO[13] = {
    "كانون الثاني", "شباط",        "آذار", "نيسان", "نوار",
    "حزيران",       "تموز",        "آب",   "أيلول", "تشرين الأول",
    "تشرين الثاني", "كانون الأول", NullS};
static const char *my_locale_ab_month_names_ar_JO[13] = {
    "كانون الثاني", "شباط",        "آذار", "نيسان", "نوار",
    "حزيران",       "تموز",        "آب",   "أيلول", "تشرين الأول",
    "تشرين الثاني", "كانون الأول", NullS};
static const char *my_locale_day_names_ar_JO[8] = {
    "الاثنين", "الثلاثاء", "الأربعاء", "الخميس",
    "الجمعة",  "السبت",    "الأحد",    NullS};
static const char *my_locale_ab_day_names_ar_JO[8] = {
    "الاثنين", "الثلاثاء", "الأربعاء", "الخميس",
    "الجمعة",  "السبت",    "الأحد",    NullS};
static TYPELIB my_locale_typelib_month_names_ar_JO = {
    array_elements(my_locale_month_names_ar_JO) - 1, "",
    my_locale_month_names_ar_JO, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ar_JO = {
    array_elements(my_locale_ab_month_names_ar_JO) - 1, "",
    my_locale_ab_month_names_ar_JO, nullptr};
static TYPELIB my_locale_typelib_day_names_ar_JO = {
    array_elements(my_locale_day_names_ar_JO) - 1, "",
    my_locale_day_names_ar_JO, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ar_JO = {
    array_elements(my_locale_ab_day_names_ar_JO) - 1, "",
    my_locale_ab_day_names_ar_JO, nullptr};
MY_LOCALE my_locale_ar_JO(8, "ar_JO", "Arabic - Jordan", false,
                          &my_locale_typelib_month_names_ar_JO,
                          &my_locale_typelib_ab_month_names_ar_JO,
                          &my_locale_typelib_day_names_ar_JO,
                          &my_locale_typelib_ab_day_names_ar_JO, 12, 8,
                          '.',    /* decimal point ar_JO */
                          ',',    /* thousands_sep ar_JO */
                          "\x03", /* grouping      ar_JO */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_JO *****/

/***** LOCALE BEGIN ar_SA: Arabic - Saudi Arabia *****/
static const char *my_locale_month_names_ar_SA[13] = {
    "كانون الثاني", "شباط",        "آذار", "نيسـان", "أيار",
    "حزيران",       "تـمـوز",      "آب",   "أيلول",  "تشرين الأول",
    "تشرين الثاني", "كانون الأول", NullS};
static const char *my_locale_ab_month_names_ar_SA[13] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
    "Aug", "Sep", "Oct", "Nov", "Dec", NullS};
static const char *my_locale_day_names_ar_SA[8] = {
    "الإثنين", "الثلاثاء", "الأربعاء", "الخميس",
    "الجمعـة", "السبت",    "الأحد",    NullS};
static const char *my_locale_ab_day_names_ar_SA[8] = {
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun", NullS};
static TYPELIB my_locale_typelib_month_names_ar_SA = {
    array_elements(my_locale_month_names_ar_SA) - 1, "",
    my_locale_month_names_ar_SA, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ar_SA = {
    array_elements(my_locale_ab_month_names_ar_SA) - 1, "",
    my_locale_ab_month_names_ar_SA, nullptr};
static TYPELIB my_locale_typelib_day_names_ar_SA = {
    array_elements(my_locale_day_names_ar_SA) - 1, "",
    my_locale_day_names_ar_SA, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ar_SA = {
    array_elements(my_locale_ab_day_names_ar_SA) - 1, "",
    my_locale_ab_day_names_ar_SA, nullptr};
MY_LOCALE my_locale_ar_SA(9, "ar_SA", "Arabic - Saudi Arabia", false,
                          &my_locale_typelib_month_names_ar_SA,
                          &my_locale_typelib_ab_month_names_ar_SA,
                          &my_locale_typelib_day_names_ar_SA,
                          &my_locale_typelib_ab_day_names_ar_SA, 12, 8,
                          '.',    /* decimal point ar_SA */
                          '\0',   /* thousands_sep ar_SA */
                          "\x80", /* grouping      ar_SA */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_SA *****/

/***** LOCALE BEGIN ar_SY: Arabic - Syria *****/
static const char *my_locale_month_names_ar_SY[13] = {
    "كانون الثاني", "شباط",        "آذار", "نيسان", "نواران",
    "حزير",         "تموز",        "آب",   "أيلول", "تشرين الأول",
    "تشرين الثاني", "كانون الأول", NullS};
static const char *my_locale_ab_month_names_ar_SY[13] = {
    "كانون الثاني", "شباط",        "آذار", "نيسان", "نوار",
    "حزيران",       "تموز",        "آب",   "أيلول", "تشرين الأول",
    "تشرين الثاني", "كانون الأول", NullS};
static const char *my_locale_day_names_ar_SY[8] = {
    "الاثنين", "الثلاثاء", "الأربعاء", "الخميس",
    "الجمعة",  "السبت",    "الأحد",    NullS};
static const char *my_locale_ab_day_names_ar_SY[8] = {
    "الاثنين", "الثلاثاء", "الأربعاء", "الخميس",
    "الجمعة",  "السبت",    "الأحد",    NullS};
static TYPELIB my_locale_typelib_month_names_ar_SY = {
    array_elements(my_locale_month_names_ar_SY) - 1, "",
    my_locale_month_names_ar_SY, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ar_SY = {
    array_elements(my_locale_ab_month_names_ar_SY) - 1, "",
    my_locale_ab_month_names_ar_SY, nullptr};
static TYPELIB my_locale_typelib_day_names_ar_SY = {
    array_elements(my_locale_day_names_ar_SY) - 1, "",
    my_locale_day_names_ar_SY, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ar_SY = {
    array_elements(my_locale_ab_day_names_ar_SY) - 1, "",
    my_locale_ab_day_names_ar_SY, nullptr};
MY_LOCALE my_locale_ar_SY(10, "ar_SY", "Arabic - Syria", false,
                          &my_locale_typelib_month_names_ar_SY,
                          &my_locale_typelib_ab_month_names_ar_SY,
                          &my_locale_typelib_day_names_ar_SY,
                          &my_locale_typelib_ab_day_names_ar_SY, 12, 8,
                          '.',    /* decimal point ar_SY */
                          ',',    /* thousands_sep ar_SY */
                          "\x03", /* grouping      ar_SY */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_SY *****/

/***** LOCALE BEGIN be_BY: Belarusian - Belarus *****/
static const char *my_locale_month_names_be_BY[13] = {
    "Студзень", "Люты",    "Сакавік", "Красавік", "Травень",
    "Чэрвень",  "Ліпень",  "Жнівень", "Верасень", "Кастрычнік",
    "Лістапад", "Снежань", NullS};
static const char *my_locale_ab_month_names_be_BY[13] = {
    "Стд", "Лют", "Сак", "Крс", "Тра", "Чэр", "Ліп",
    "Жнв", "Врс", "Кст", "Ліс", "Снж", NullS};
static const char *my_locale_day_names_be_BY[8] = {
    "Панядзелак", "Аўторак", "Серада",  "Чацвер",
    "Пятніца",    "Субота",  "Нядзеля", NullS};
static const char *my_locale_ab_day_names_be_BY[8] = {
    "Пан", "Аўт", "Срд", "Чцв", "Пят", "Суб", "Няд", NullS};
static TYPELIB my_locale_typelib_month_names_be_BY = {
    array_elements(my_locale_month_names_be_BY) - 1, "",
    my_locale_month_names_be_BY, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_be_BY = {
    array_elements(my_locale_ab_month_names_be_BY) - 1, "",
    my_locale_ab_month_names_be_BY, nullptr};
static TYPELIB my_locale_typelib_day_names_be_BY = {
    array_elements(my_locale_day_names_be_BY) - 1, "",
    my_locale_day_names_be_BY, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_be_BY = {
    array_elements(my_locale_ab_day_names_be_BY) - 1, "",
    my_locale_ab_day_names_be_BY, nullptr};
MY_LOCALE my_locale_be_BY(11, "be_BY", "Belarusian - Belarus", false,
                          &my_locale_typelib_month_names_be_BY,
                          &my_locale_typelib_ab_month_names_be_BY,
                          &my_locale_typelib_day_names_be_BY,
                          &my_locale_typelib_ab_day_names_be_BY, 10, 10,
                          ',',        /* decimal point be_BY */
                          '.',        /* thousands_sep be_BY */
                          "\x03\x03", /* grouping      be_BY */
                          &global_errmsgs[en_US]);
/***** LOCALE END be_BY *****/

/***** LOCALE BEGIN bg_BG: Bulgarian - Bulgaria *****/
static const char *my_locale_month_names_bg_BG[13] = {
    "януари", "февруари",  "март",     "април",   "май",      "юни", "юли",
    "август", "септември", "октомври", "ноември", "декември", NullS};
static const char *my_locale_ab_month_names_bg_BG[13] = {
    "яну", "фев", "мар", "апр", "май", "юни", "юли",
    "авг", "сеп", "окт", "ное", "дек", NullS};
static const char *my_locale_day_names_bg_BG[8] = {
    "понеделник", "вторник", "сряда",  "четвъртък",
    "петък",      "събота",  "неделя", NullS};
static const char *my_locale_ab_day_names_bg_BG[8] = {"пн", "вт", "ср", "чт",
                                                      "пт", "сб", "нд", NullS};
static TYPELIB my_locale_typelib_month_names_bg_BG = {
    array_elements(my_locale_month_names_bg_BG) - 1, "",
    my_locale_month_names_bg_BG, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_bg_BG = {
    array_elements(my_locale_ab_month_names_bg_BG) - 1, "",
    my_locale_ab_month_names_bg_BG, nullptr};
static TYPELIB my_locale_typelib_day_names_bg_BG = {
    array_elements(my_locale_day_names_bg_BG) - 1, "",
    my_locale_day_names_bg_BG, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_bg_BG = {
    array_elements(my_locale_ab_day_names_bg_BG) - 1, "",
    my_locale_ab_day_names_bg_BG, nullptr};
MY_LOCALE my_locale_bg_BG(12, "bg_BG", "Bulgarian - Bulgaria", false,
                          &my_locale_typelib_month_names_bg_BG,
                          &my_locale_typelib_ab_month_names_bg_BG,
                          &my_locale_typelib_day_names_bg_BG,
                          &my_locale_typelib_ab_day_names_bg_BG, 9, 10,
                          ',',        /* decimal point bg_BG */
                          '\0',       /* thousands_sep bg_BG */
                          "\x03\x03", /* grouping      bg_BG */
                          &global_errmsgs[en_US]);
/***** LOCALE END bg_BG *****/

/***** LOCALE BEGIN ca_ES: Catalan - Catalan *****/
static const char *my_locale_month_names_ca_ES[13] = {
    "gener", "febrer",   "març",    "abril",    "maig",     "juny", "juliol",
    "agost", "setembre", "octubre", "novembre", "desembre", NullS};
static const char *my_locale_ab_month_names_ca_ES[13] = {
    "gen", "feb", "mar", "abr", "mai", "jun", "jul",
    "ago", "set", "oct", "nov", "des", NullS};
static const char *my_locale_day_names_ca_ES[8] = {
    "dilluns",   "dimarts",  "dimecres", "dijous",
    "divendres", "dissabte", "diumenge", NullS};
static const char *my_locale_ab_day_names_ca_ES[8] = {"dl", "dt", "dc", "dj",
                                                      "dv", "ds", "dg", NullS};
static TYPELIB my_locale_typelib_month_names_ca_ES = {
    array_elements(my_locale_month_names_ca_ES) - 1, "",
    my_locale_month_names_ca_ES, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ca_ES = {
    array_elements(my_locale_ab_month_names_ca_ES) - 1, "",
    my_locale_ab_month_names_ca_ES, nullptr};
static TYPELIB my_locale_typelib_day_names_ca_ES = {
    array_elements(my_locale_day_names_ca_ES) - 1, "",
    my_locale_day_names_ca_ES, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ca_ES = {
    array_elements(my_locale_ab_day_names_ca_ES) - 1, "",
    my_locale_ab_day_names_ca_ES, nullptr};
MY_LOCALE my_locale_ca_ES(13, "ca_ES", "Catalan - Catalan", false,
                          &my_locale_typelib_month_names_ca_ES,
                          &my_locale_typelib_ab_month_names_ca_ES,
                          &my_locale_typelib_day_names_ca_ES,
                          &my_locale_typelib_ab_day_names_ca_ES, 8, 9,
                          ',',        /* decimal point ca_ES */
                          '\0',       /* thousands_sep ca_ES */
                          "\x80\x80", /* grouping      ca_ES */
                          &global_errmsgs[en_US]);
/***** LOCALE END ca_ES *****/

/***** LOCALE BEGIN cs_CZ: Czech - Czech Republic *****/
static const char *my_locale_month_names_cs_CZ[13] = {
    "leden", "únor", "březen", "duben",    "květen",   "červen", "červenec",
    "srpen", "září", "říjen",  "listopad", "prosinec", NullS};
static const char *my_locale_ab_month_names_cs_CZ[13] = {
    "led", "úno", "bře", "dub", "kvě", "čen", "čec",
    "srp", "zář", "říj", "lis", "pro", NullS};
static const char *my_locale_day_names_cs_CZ[8] = {"Pondělí", "Úterý", "Středa",
                                                   "Čtvrtek", "Pátek", "Sobota",
                                                   "Neděle",  NullS};
static const char *my_locale_ab_day_names_cs_CZ[8] = {"Po", "Út", "St", "Čt",
                                                      "Pá", "So", "Ne", NullS};
static TYPELIB my_locale_typelib_month_names_cs_CZ = {
    array_elements(my_locale_month_names_cs_CZ) - 1, "",
    my_locale_month_names_cs_CZ, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_cs_CZ = {
    array_elements(my_locale_ab_month_names_cs_CZ) - 1, "",
    my_locale_ab_month_names_cs_CZ, nullptr};
static TYPELIB my_locale_typelib_day_names_cs_CZ = {
    array_elements(my_locale_day_names_cs_CZ) - 1, "",
    my_locale_day_names_cs_CZ, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_cs_CZ = {
    array_elements(my_locale_ab_day_names_cs_CZ) - 1, "",
    my_locale_ab_day_names_cs_CZ, nullptr};
MY_LOCALE my_locale_cs_CZ(14, "cs_CZ", "Czech - Czech Republic", false,
                          &my_locale_typelib_month_names_cs_CZ,
                          &my_locale_typelib_ab_month_names_cs_CZ,
                          &my_locale_typelib_day_names_cs_CZ,
                          &my_locale_typelib_ab_day_names_cs_CZ, 8, 7,
                          ',',        /* decimal point cs_CZ */
                          ' ',        /* thousands_sep cs_CZ */
                          "\x03\x03", /* grouping      cs_CZ */
                          &global_errmsgs[cs_CZ]);
/***** LOCALE END cs_CZ *****/

/***** LOCALE BEGIN da_DK: Danish - Denmark *****/
static const char *my_locale_month_names_da_DK[13] = {
    "januar", "februar",   "marts",   "april",    "maj",      "juni", "juli",
    "august", "september", "oktober", "november", "december", NullS};
static const char *my_locale_ab_month_names_da_DK[13] = {
    "jan", "feb", "mar", "apr", "maj", "jun", "jul",
    "aug", "sep", "okt", "nov", "dec", NullS};
static const char *my_locale_day_names_da_DK[8] = {
    "mandag", "tirsdag", "onsdag", "torsdag",
    "fredag", "lørdag",  "søndag", NullS};
static const char *my_locale_ab_day_names_da_DK[8] = {
    "man", "tir", "ons", "tor", "fre", "lør", "søn", NullS};
static TYPELIB my_locale_typelib_month_names_da_DK = {
    array_elements(my_locale_month_names_da_DK) - 1, "",
    my_locale_month_names_da_DK, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_da_DK = {
    array_elements(my_locale_ab_month_names_da_DK) - 1, "",
    my_locale_ab_month_names_da_DK, nullptr};
static TYPELIB my_locale_typelib_day_names_da_DK = {
    array_elements(my_locale_day_names_da_DK) - 1, "",
    my_locale_day_names_da_DK, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_da_DK = {
    array_elements(my_locale_ab_day_names_da_DK) - 1, "",
    my_locale_ab_day_names_da_DK, nullptr};
MY_LOCALE my_locale_da_DK(15, "da_DK", "Danish - Denmark", false,
                          &my_locale_typelib_month_names_da_DK,
                          &my_locale_typelib_ab_month_names_da_DK,
                          &my_locale_typelib_day_names_da_DK,
                          &my_locale_typelib_ab_day_names_da_DK, 9, 7,
                          ',',        /* decimal point da_DK */
                          '.',        /* thousands_sep da_DK */
                          "\x03\x03", /* grouping      da_DK */
                          &global_errmsgs[da_DK]);
/***** LOCALE END da_DK *****/

/***** LOCALE BEGIN de_AT: German - Austria *****/
static const char *my_locale_month_names_de_AT[13] = {
    "Jänner", "Feber",     "März",    "April",    "Mai",      "Juni", "Juli",
    "August", "September", "Oktober", "November", "Dezember", NullS};
static const char *my_locale_ab_month_names_de_AT[13] = {
    "Jän", "Feb", "Mär", "Apr", "Mai", "Jun", "Jul",
    "Aug", "Sep", "Okt", "Nov", "Dez", NullS};
static const char *my_locale_day_names_de_AT[8] = {
    "Montag",  "Dienstag", "Mittwoch", "Donnerstag",
    "Freitag", "Samstag",  "Sonntag",  NullS};
static const char *my_locale_ab_day_names_de_AT[8] = {
    "Mon", "Die", "Mit", "Don", "Fre", "Sam", "Son", NullS};
static TYPELIB my_locale_typelib_month_names_de_AT = {
    array_elements(my_locale_month_names_de_AT) - 1, "",
    my_locale_month_names_de_AT, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_de_AT = {
    array_elements(my_locale_ab_month_names_de_AT) - 1, "",
    my_locale_ab_month_names_de_AT, nullptr};
static TYPELIB my_locale_typelib_day_names_de_AT = {
    array_elements(my_locale_day_names_de_AT) - 1, "",
    my_locale_day_names_de_AT, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_de_AT = {
    array_elements(my_locale_ab_day_names_de_AT) - 1, "",
    my_locale_ab_day_names_de_AT, nullptr};
MY_LOCALE my_locale_de_AT(16, "de_AT", "German - Austria", false,
                          &my_locale_typelib_month_names_de_AT,
                          &my_locale_typelib_ab_month_names_de_AT,
                          &my_locale_typelib_day_names_de_AT,
                          &my_locale_typelib_ab_day_names_de_AT, 9, 10,
                          ',',        /* decimal point de_AT */
                          '\0',       /* thousands_sep de_AT */
                          "\x80\x80", /* grouping      de_AT */
                          &global_errmsgs[de_DE]);
/***** LOCALE END de_AT *****/

/***** LOCALE BEGIN de_DE: German - Germany *****/
static const char *my_locale_month_names_de_DE[13] = {
    "Januar", "Februar",   "März",    "April",    "Mai",      "Juni", "Juli",
    "August", "September", "Oktober", "November", "Dezember", NullS};
static const char *my_locale_ab_month_names_de_DE[13] = {
    "Jan", "Feb", "Mär", "Apr", "Mai", "Jun", "Jul",
    "Aug", "Sep", "Okt", "Nov", "Dez", NullS};
static const char *my_locale_day_names_de_DE[8] = {
    "Montag",  "Dienstag", "Mittwoch", "Donnerstag",
    "Freitag", "Samstag",  "Sonntag",  NullS};
static const char *my_locale_ab_day_names_de_DE[8] = {"Mo", "Di", "Mi", "Do",
                                                      "Fr", "Sa", "So", NullS};
static TYPELIB my_locale_typelib_month_names_de_DE = {
    array_elements(my_locale_month_names_de_DE) - 1, "",
    my_locale_month_names_de_DE, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_de_DE = {
    array_elements(my_locale_ab_month_names_de_DE) - 1, "",
    my_locale_ab_month_names_de_DE, nullptr};
static TYPELIB my_locale_typelib_day_names_de_DE = {
    array_elements(my_locale_day_names_de_DE) - 1, "",
    my_locale_day_names_de_DE, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_de_DE = {
    array_elements(my_locale_ab_day_names_de_DE) - 1, "",
    my_locale_ab_day_names_de_DE, nullptr};
MY_LOCALE my_locale_de_DE(4, "de_DE", "German - Germany", false,
                          &my_locale_typelib_month_names_de_DE,
                          &my_locale_typelib_ab_month_names_de_DE,
                          &my_locale_typelib_day_names_de_DE,
                          &my_locale_typelib_ab_day_names_de_DE, 9, 10,
                          ',',        /* decimal point de_DE */
                          '.',        /* thousands_sep de_DE */
                          "\x03\x03", /* grouping      de_DE */
                          &global_errmsgs[de_DE]);
/***** LOCALE END de_DE *****/

/***** LOCALE BEGIN en_US: English - United States *****/
static const char *my_locale_month_names_en_US[13] = {
    "January", "February",  "March",   "April",    "May",      "June", "July",
    "August",  "September", "October", "November", "December", NullS};
static const char *my_locale_ab_month_names_en_US[13] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
    "Aug", "Sep", "Oct", "Nov", "Dec", NullS};
static const char *my_locale_day_names_en_US[8] = {
    "Monday", "Tuesday",  "Wednesday", "Thursday",
    "Friday", "Saturday", "Sunday",    NullS};
static const char *my_locale_ab_day_names_en_US[8] = {
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun", NullS};
static TYPELIB my_locale_typelib_month_names_en_US = {
    array_elements(my_locale_month_names_en_US) - 1, "",
    my_locale_month_names_en_US, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_en_US = {
    array_elements(my_locale_ab_month_names_en_US) - 1, "",
    my_locale_ab_month_names_en_US, nullptr};
static TYPELIB my_locale_typelib_day_names_en_US = {
    array_elements(my_locale_day_names_en_US) - 1, "",
    my_locale_day_names_en_US, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_en_US = {
    array_elements(my_locale_ab_day_names_en_US) - 1, "",
    my_locale_ab_day_names_en_US, nullptr};
MY_LOCALE my_locale_en_US(0, "en_US", "English - United States", true,
                          &my_locale_typelib_month_names_en_US,
                          &my_locale_typelib_ab_month_names_en_US,
                          &my_locale_typelib_day_names_en_US,
                          &my_locale_typelib_ab_day_names_en_US, 9, 9,
                          '.',        /* decimal point en_US */
                          ',',        /* thousands_sep en_US */
                          "\x03\x03", /* grouping      en_US */
                          &global_errmsgs[en_US]);
/***** LOCALE END en_US *****/

/***** LOCALE BEGIN es_ES: Spanish - Spain *****/
static const char *my_locale_month_names_es_ES[13] = {
    "enero",     "febrero",   "marzo",  "abril",      "mayo",
    "junio",     "julio",     "agosto", "septiembre", "octubre",
    "noviembre", "diciembre", NullS};
static const char *my_locale_ab_month_names_es_ES[13] = {
    "ene", "feb", "mar", "abr", "may", "jun", "jul",
    "ago", "sep", "oct", "nov", "dic", NullS};
static const char *my_locale_day_names_es_ES[8] = {
    "lunes",   "martes", "miércoles", "jueves",
    "viernes", "sábado", "domingo",   NullS};
static const char *my_locale_ab_day_names_es_ES[8] = {
    "lun", "mar", "mié", "jue", "vie", "sáb", "dom", NullS};
static TYPELIB my_locale_typelib_month_names_es_ES = {
    array_elements(my_locale_month_names_es_ES) - 1, "",
    my_locale_month_names_es_ES, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_es_ES = {
    array_elements(my_locale_ab_month_names_es_ES) - 1, "",
    my_locale_ab_month_names_es_ES, nullptr};
static TYPELIB my_locale_typelib_day_names_es_ES = {
    array_elements(my_locale_day_names_es_ES) - 1, "",
    my_locale_day_names_es_ES, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_es_ES = {
    array_elements(my_locale_ab_day_names_es_ES) - 1, "",
    my_locale_ab_day_names_es_ES, nullptr};
MY_LOCALE my_locale_es_ES(17, "es_ES", "Spanish - Spain", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          ',',        /* decimal point es_ES */
                          '\0',       /* thousands_sep es_ES */
                          "\x80\x80", /* grouping      es_ES */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_ES *****/

/***** LOCALE BEGIN et_EE: Estonian - Estonia *****/
static const char *my_locale_month_names_et_EE[13] = {
    "jaanuar",  "veebruar",  "märts",  "aprill",    "mai",
    "juuni",    "juuli",     "august", "september", "oktoober",
    "november", "detsember", NullS};
static const char *my_locale_ab_month_names_et_EE[13] = {
    "jaan ", "veebr", "märts", "apr  ", "mai  ", "juuni", "juuli",
    "aug  ", "sept ", "okt  ", "nov  ", "dets ", NullS};
static const char *my_locale_day_names_et_EE[8] = {
    "esmaspäev", "teisipäev", "kolmapäev", "neljapäev",
    "reede",     "laupäev",   "pühapäev",  NullS};
static const char *my_locale_ab_day_names_et_EE[8] = {"E", "T", "K", "N",
                                                      "R", "L", "P", NullS};
static TYPELIB my_locale_typelib_month_names_et_EE = {
    array_elements(my_locale_month_names_et_EE) - 1, "",
    my_locale_month_names_et_EE, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_et_EE = {
    array_elements(my_locale_ab_month_names_et_EE) - 1, "",
    my_locale_ab_month_names_et_EE, nullptr};
static TYPELIB my_locale_typelib_day_names_et_EE = {
    array_elements(my_locale_day_names_et_EE) - 1, "",
    my_locale_day_names_et_EE, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_et_EE = {
    array_elements(my_locale_ab_day_names_et_EE) - 1, "",
    my_locale_ab_day_names_et_EE, nullptr};
MY_LOCALE my_locale_et_EE(18, "et_EE", "Estonian - Estonia", false,
                          &my_locale_typelib_month_names_et_EE,
                          &my_locale_typelib_ab_month_names_et_EE,
                          &my_locale_typelib_day_names_et_EE,
                          &my_locale_typelib_ab_day_names_et_EE, 9, 9,
                          ',',        /* decimal point et_EE */
                          ' ',        /* thousands_sep et_EE */
                          "\x03\x03", /* grouping      et_EE */
                          &global_errmsgs[et_EE]);
/***** LOCALE END et_EE *****/

/***** LOCALE BEGIN eu_ES: Basque - Basque *****/
static const char *my_locale_month_names_eu_ES[13] = {
    "urtarrila", "otsaila", "martxoa", "apirila", "maiatza",
    "ekaina",    "uztaila", "abuztua", "iraila",  "urria",
    "azaroa",    "abendua", NullS};
static const char *my_locale_ab_month_names_eu_ES[13] = {
    "urt", "ots", "mar", "api", "mai", "eka", "uzt",
    "abu", "ira", "urr", "aza", "abe", NullS};
static const char *my_locale_day_names_eu_ES[8] = {
    "astelehena", "asteartea", "asteazkena", "osteguna",
    "ostirala",   "larunbata", "igandea",    NullS};
static const char *my_locale_ab_day_names_eu_ES[8] = {
    "al.", "ar.", "az.", "og.", "or.", "lr.", "ig.", NullS};
static TYPELIB my_locale_typelib_month_names_eu_ES = {
    array_elements(my_locale_month_names_eu_ES) - 1, "",
    my_locale_month_names_eu_ES, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_eu_ES = {
    array_elements(my_locale_ab_month_names_eu_ES) - 1, "",
    my_locale_ab_month_names_eu_ES, nullptr};
static TYPELIB my_locale_typelib_day_names_eu_ES = {
    array_elements(my_locale_day_names_eu_ES) - 1, "",
    my_locale_day_names_eu_ES, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_eu_ES = {
    array_elements(my_locale_ab_day_names_eu_ES) - 1, "",
    my_locale_ab_day_names_eu_ES, nullptr};
MY_LOCALE my_locale_eu_ES(19, "eu_ES", "Basque - Basque", true,
                          &my_locale_typelib_month_names_eu_ES,
                          &my_locale_typelib_ab_month_names_eu_ES,
                          &my_locale_typelib_day_names_eu_ES,
                          &my_locale_typelib_ab_day_names_eu_ES, 9, 10,
                          ',',        /* decimal point eu_ES */
                          '\0',       /* thousands_sep eu_ES */
                          "\x80\x80", /* grouping      eu_ES */
                          &global_errmsgs[en_US]);
/***** LOCALE END eu_ES *****/

/***** LOCALE BEGIN fi_FI: Finnish - Finland *****/
static const char *my_locale_month_names_fi_FI[13] = {
    "tammikuu",  "helmikuu", "maaliskuu", "huhtikuu", "toukokuu",
    "kesäkuu",   "heinäkuu", "elokuu",    "syyskuu",  "lokakuu",
    "marraskuu", "joulukuu", NullS};
static const char *my_locale_ab_month_names_fi_FI[13] = {
    "tammi ", "helmi ", "maalis", "huhti ", "touko ", "kesä  ", "heinä ",
    "elo   ", "syys  ", "loka  ", "marras", "joulu ", NullS};
static const char *my_locale_day_names_fi_FI[8] = {
    "maanantai", "tiistai",  "keskiviikko", "torstai",
    "perjantai", "lauantai", "sunnuntai",   NullS};
static const char *my_locale_ab_day_names_fi_FI[8] = {"ma", "ti", "ke", "to",
                                                      "pe", "la", "su", NullS};
static TYPELIB my_locale_typelib_month_names_fi_FI = {
    array_elements(my_locale_month_names_fi_FI) - 1, "",
    my_locale_month_names_fi_FI, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_fi_FI = {
    array_elements(my_locale_ab_month_names_fi_FI) - 1, "",
    my_locale_ab_month_names_fi_FI, nullptr};
static TYPELIB my_locale_typelib_day_names_fi_FI = {
    array_elements(my_locale_day_names_fi_FI) - 1, "",
    my_locale_day_names_fi_FI, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_fi_FI = {
    array_elements(my_locale_ab_day_names_fi_FI) - 1, "",
    my_locale_ab_day_names_fi_FI, nullptr};
MY_LOCALE my_locale_fi_FI(20, "fi_FI", "Finnish - Finland", false,
                          &my_locale_typelib_month_names_fi_FI,
                          &my_locale_typelib_ab_month_names_fi_FI,
                          &my_locale_typelib_day_names_fi_FI,
                          &my_locale_typelib_ab_day_names_fi_FI, 9, 11,
                          ',',        /* decimal point fi_FI */
                          ' ',        /* thousands_sep fi_FI */
                          "\x03\x03", /* grouping      fi_FI */
                          &global_errmsgs[en_US]);
/***** LOCALE END fi_FI *****/

/***** LOCALE BEGIN fo_FO: Faroese - Faroe Islands *****/
static const char *my_locale_month_names_fo_FO[13] = {
    "januar", "februar",   "mars",    "apríl",    "mai",      "juni", "juli",
    "august", "september", "oktober", "november", "desember", NullS};
static const char *my_locale_ab_month_names_fo_FO[13] = {
    "jan", "feb", "mar", "apr", "mai", "jun", "jul",
    "aug", "sep", "okt", "nov", "des", NullS};
static const char *my_locale_day_names_fo_FO[8] = {
    "mánadagur",    "týsdagur",    "mikudagur",  "hósdagur",
    "fríggjadagur", "leygardagur", "sunnudagur", NullS};
static const char *my_locale_ab_day_names_fo_FO[8] = {
    "mán", "týs", "mik", "hós", "frí", "ley", "sun", NullS};
static TYPELIB my_locale_typelib_month_names_fo_FO = {
    array_elements(my_locale_month_names_fo_FO) - 1, "",
    my_locale_month_names_fo_FO, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_fo_FO = {
    array_elements(my_locale_ab_month_names_fo_FO) - 1, "",
    my_locale_ab_month_names_fo_FO, nullptr};
static TYPELIB my_locale_typelib_day_names_fo_FO = {
    array_elements(my_locale_day_names_fo_FO) - 1, "",
    my_locale_day_names_fo_FO, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_fo_FO = {
    array_elements(my_locale_ab_day_names_fo_FO) - 1, "",
    my_locale_ab_day_names_fo_FO, nullptr};
MY_LOCALE my_locale_fo_FO(21, "fo_FO", "Faroese - Faroe Islands", false,
                          &my_locale_typelib_month_names_fo_FO,
                          &my_locale_typelib_ab_month_names_fo_FO,
                          &my_locale_typelib_day_names_fo_FO,
                          &my_locale_typelib_ab_day_names_fo_FO, 9, 12,
                          ',',        /* decimal point fo_FO */
                          '.',        /* thousands_sep fo_FO */
                          "\x03\x03", /* grouping      fo_FO */
                          &global_errmsgs[en_US]);
/***** LOCALE END fo_FO *****/

/***** LOCALE BEGIN fr_FR: French - France *****/
static const char *my_locale_month_names_fr_FR[13] = {
    "janvier",  "février",  "mars", "avril",     "mai",
    "juin",     "juillet",  "août", "septembre", "octobre",
    "novembre", "décembre", NullS};
static const char *my_locale_ab_month_names_fr_FR[13] = {
    "jan", "fév", "mar", "avr", "mai", "jun", "jui",
    "aoû", "sep", "oct", "nov", "déc", NullS};
static const char *my_locale_day_names_fr_FR[8] = {
    "lundi",    "mardi",  "mercredi", "jeudi",
    "vendredi", "samedi", "dimanche", NullS};
static const char *my_locale_ab_day_names_fr_FR[8] = {
    "lun", "mar", "mer", "jeu", "ven", "sam", "dim", NullS};
static TYPELIB my_locale_typelib_month_names_fr_FR = {
    array_elements(my_locale_month_names_fr_FR) - 1, "",
    my_locale_month_names_fr_FR, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_fr_FR = {
    array_elements(my_locale_ab_month_names_fr_FR) - 1, "",
    my_locale_ab_month_names_fr_FR, nullptr};
static TYPELIB my_locale_typelib_day_names_fr_FR = {
    array_elements(my_locale_day_names_fr_FR) - 1, "",
    my_locale_day_names_fr_FR, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_fr_FR = {
    array_elements(my_locale_ab_day_names_fr_FR) - 1, "",
    my_locale_ab_day_names_fr_FR, nullptr};
MY_LOCALE my_locale_fr_FR(5, "fr_FR", "French - France", false,
                          &my_locale_typelib_month_names_fr_FR,
                          &my_locale_typelib_ab_month_names_fr_FR,
                          &my_locale_typelib_day_names_fr_FR,
                          &my_locale_typelib_ab_day_names_fr_FR, 9, 8,
                          ',',        /* decimal point fr_FR */
                          '\0',       /* thousands_sep fr_FR */
                          "\x80\x80", /* grouping      fr_FR */
                          &global_errmsgs[fr_FR]);
/***** LOCALE END fr_FR *****/

/***** LOCALE BEGIN gl_ES: Galician - Galician *****/
static const char *my_locale_month_names_gl_ES[13] = {
    "Xaneiro", "Febreiro", "Marzo",   "Abril",    "Maio",     "Xuño", "Xullo",
    "Agosto",  "Setembro", "Outubro", "Novembro", "Decembro", NullS};
static const char *my_locale_ab_month_names_gl_ES[13] = {
    "Xan", "Feb", "Mar", "Abr", "Mai", "Xuñ", "Xul",
    "Ago", "Set", "Out", "Nov", "Dec", NullS};
static const char *my_locale_day_names_gl_ES[8] = {
    "Luns",   "Martes", "Mércores", "Xoves",
    "Venres", "Sábado", "Domingo",  NullS};
static const char *my_locale_ab_day_names_gl_ES[8] = {
    "Lun", "Mar", "Mér", "Xov", "Ven", "Sáb", "Dom", NullS};
static TYPELIB my_locale_typelib_month_names_gl_ES = {
    array_elements(my_locale_month_names_gl_ES) - 1, "",
    my_locale_month_names_gl_ES, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_gl_ES = {
    array_elements(my_locale_ab_month_names_gl_ES) - 1, "",
    my_locale_ab_month_names_gl_ES, nullptr};
static TYPELIB my_locale_typelib_day_names_gl_ES = {
    array_elements(my_locale_day_names_gl_ES) - 1, "",
    my_locale_day_names_gl_ES, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_gl_ES = {
    array_elements(my_locale_ab_day_names_gl_ES) - 1, "",
    my_locale_ab_day_names_gl_ES, nullptr};
MY_LOCALE my_locale_gl_ES(22, "gl_ES", "Galician - Galician", false,
                          &my_locale_typelib_month_names_gl_ES,
                          &my_locale_typelib_ab_month_names_gl_ES,
                          &my_locale_typelib_day_names_gl_ES,
                          &my_locale_typelib_ab_day_names_gl_ES, 8, 8,
                          ',',        /* decimal point gl_ES */
                          '\0',       /* thousands_sep gl_ES */
                          "\x80\x80", /* grouping      gl_ES */
                          &global_errmsgs[en_US]);
/***** LOCALE END gl_ES *****/

/***** LOCALE BEGIN gu_IN: Gujarati - India *****/
static const char *my_locale_month_names_gu_IN[13] = {
    "જાન્યુઆરી", "ફેબ્રુઆરી", "માર્ચ",   "એપ્રિલ", "મે",      "જુન", "જુલાઇ",
    "ઓગસ્ટ",    "સેપ્ટેમ્બર", "ઓક્ટોબર", "નવેમ્બર", "ડિસેમ્બર", NullS};
static const char *my_locale_ab_month_names_gu_IN[13] = {
    "જાન", "ફેબ",  "માર", "એપ્ર", "મે",   "જુન", "જુલ",
    "ઓગ",  "સેપ્ટ", "ઓક્ટ", "નોવ", "ડિસ", NullS};
static const char *my_locale_day_names_gu_IN[8] = {
    "સોમવાર", "મન્ગળવાર", "બુધવાર", "ગુરુવાર", "શુક્રવાર", "શનિવાર", "રવિવાર", NullS};
static const char *my_locale_ab_day_names_gu_IN[8] = {
    "સોમ", "મન્ગળ", "બુધ", "ગુરુ", "શુક્ર", "શનિ", "રવિ", NullS};
static TYPELIB my_locale_typelib_month_names_gu_IN = {
    array_elements(my_locale_month_names_gu_IN) - 1, "",
    my_locale_month_names_gu_IN, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_gu_IN = {
    array_elements(my_locale_ab_month_names_gu_IN) - 1, "",
    my_locale_ab_month_names_gu_IN, nullptr};
static TYPELIB my_locale_typelib_day_names_gu_IN = {
    array_elements(my_locale_day_names_gu_IN) - 1, "",
    my_locale_day_names_gu_IN, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_gu_IN = {
    array_elements(my_locale_ab_day_names_gu_IN) - 1, "",
    my_locale_ab_day_names_gu_IN, nullptr};
MY_LOCALE my_locale_gu_IN(23, "gu_IN", "Gujarati - India", false,
                          &my_locale_typelib_month_names_gu_IN,
                          &my_locale_typelib_ab_month_names_gu_IN,
                          &my_locale_typelib_day_names_gu_IN,
                          &my_locale_typelib_ab_day_names_gu_IN, 10, 8,
                          '.',    /* decimal point gu_IN */
                          ',',    /* thousands_sep gu_IN */
                          "\x03", /* grouping      gu_IN */
                          &global_errmsgs[en_US]);
/***** LOCALE END gu_IN *****/

/***** LOCALE BEGIN he_IL: Hebrew - Israel *****/
static const char *my_locale_month_names_he_IL[13] = {
    "ינואר",  "פברואר", "מרץ",     "אפריל",  "מאי",   "יוני", "יולי",
    "אוגוסט", "ספטמבר", "אוקטובר", "נובמבר", "דצמבר", NullS};
static const char *my_locale_ab_month_names_he_IL[13] = {
    "ינו", "פבר", "מרץ", "אפר", "מאי", "יונ", "יול",
    "אוג", "ספט", "אוק", "נוב", "דצמ", NullS};
static const char *my_locale_day_names_he_IL[8] = {
    "שני", "שלישי", "רביעי", "חמישי", "שישי", "שבת", "ראשון", NullS};
static const char *my_locale_ab_day_names_he_IL[8] = {"ב'", "ג'", "ד'", "ה'",
                                                      "ו'", "ש'", "א'", NullS};
static TYPELIB my_locale_typelib_month_names_he_IL = {
    array_elements(my_locale_month_names_he_IL) - 1, "",
    my_locale_month_names_he_IL, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_he_IL = {
    array_elements(my_locale_ab_month_names_he_IL) - 1, "",
    my_locale_ab_month_names_he_IL, nullptr};
static TYPELIB my_locale_typelib_day_names_he_IL = {
    array_elements(my_locale_day_names_he_IL) - 1, "",
    my_locale_day_names_he_IL, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_he_IL = {
    array_elements(my_locale_ab_day_names_he_IL) - 1, "",
    my_locale_ab_day_names_he_IL, nullptr};
MY_LOCALE my_locale_he_IL(24, "he_IL", "Hebrew - Israel", false,
                          &my_locale_typelib_month_names_he_IL,
                          &my_locale_typelib_ab_month_names_he_IL,
                          &my_locale_typelib_day_names_he_IL,
                          &my_locale_typelib_ab_day_names_he_IL, 7, 5,
                          '.',        /* decimal point he_IL */
                          ',',        /* thousands_sep he_IL */
                          "\x03\x03", /* grouping      he_IL */
                          &global_errmsgs[en_US]);
/***** LOCALE END he_IL *****/

/***** LOCALE BEGIN hi_IN: Hindi - India *****/
static const char *my_locale_month_names_hi_IN[13] = {
    "जनवरी", "फ़रवरी",  "मार्च",  "अप्रेल",  "मई",     "जून", "जुलाई",
    "अगस्त",  "सितम्बर", "अक्टूबर", "नवम्बर", "दिसम्बर", NullS};
static const char *my_locale_ab_month_names_hi_IN[13] = {
    "जनवरी", "फ़रवरी",  "मार्च",  "अप्रेल",  "मई",     "जून", "जुलाई",
    "अगस्त",  "सितम्बर", "अक्टूबर", "नवम्बर", "दिसम्बर", NullS};
static const char *my_locale_day_names_hi_IN[8] = {
    "सोमवार ", "मंगलवार ", "बुधवार ",  "गुरुवार ",
    "शुक्रवार ", "शनिवार ", "रविवार ", NullS};
static const char *my_locale_ab_day_names_hi_IN[8] = {
    "सोम ", "मंगल ", "बुध ", "गुरु ", "शुक्र ", "शनि ", "रवि ", NullS};
static TYPELIB my_locale_typelib_month_names_hi_IN = {
    array_elements(my_locale_month_names_hi_IN) - 1, "",
    my_locale_month_names_hi_IN, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_hi_IN = {
    array_elements(my_locale_ab_month_names_hi_IN) - 1, "",
    my_locale_ab_month_names_hi_IN, nullptr};
static TYPELIB my_locale_typelib_day_names_hi_IN = {
    array_elements(my_locale_day_names_hi_IN) - 1, "",
    my_locale_day_names_hi_IN, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_hi_IN = {
    array_elements(my_locale_ab_day_names_hi_IN) - 1, "",
    my_locale_ab_day_names_hi_IN, nullptr};
MY_LOCALE my_locale_hi_IN(25, "hi_IN", "Hindi - India", false,
                          &my_locale_typelib_month_names_hi_IN,
                          &my_locale_typelib_ab_month_names_hi_IN,
                          &my_locale_typelib_day_names_hi_IN,
                          &my_locale_typelib_ab_day_names_hi_IN, 7, 9,
                          '.',    /* decimal point hi_IN */
                          ',',    /* thousands_sep hi_IN */
                          "\x03", /* grouping      hi_IN */
                          &global_errmsgs[en_US]);
/***** LOCALE END hi_IN *****/

/***** LOCALE BEGIN hr_HR: Croatian - Croatia *****/
static const char *my_locale_month_names_hr_HR[13] = {
    "Siječanj", "Veljača",  "Ožujak",  "Travanj", "Svibanj",
    "Lipanj",   "Srpanj",   "Kolovoz", "Rujan",   "Listopad",
    "Studeni",  "Prosinac", NullS};
static const char *my_locale_ab_month_names_hr_HR[13] = {
    "Sij", "Vel", "Ožu", "Tra", "Svi", "Lip", "Srp",
    "Kol", "Ruj", "Lis", "Stu", "Pro", NullS};
static const char *my_locale_day_names_hr_HR[8] = {
    "Ponedjeljak", "Utorak", "Srijeda",  "Četvrtak",
    "Petak",       "Subota", "Nedjelja", NullS};
static const char *my_locale_ab_day_names_hr_HR[8] = {
    "Pon", "Uto", "Sri", "Čet", "Pet", "Sub", "Ned", NullS};
static TYPELIB my_locale_typelib_month_names_hr_HR = {
    array_elements(my_locale_month_names_hr_HR) - 1, "",
    my_locale_month_names_hr_HR, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_hr_HR = {
    array_elements(my_locale_ab_month_names_hr_HR) - 1, "",
    my_locale_ab_month_names_hr_HR, nullptr};
static TYPELIB my_locale_typelib_day_names_hr_HR = {
    array_elements(my_locale_day_names_hr_HR) - 1, "",
    my_locale_day_names_hr_HR, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_hr_HR = {
    array_elements(my_locale_ab_day_names_hr_HR) - 1, "",
    my_locale_ab_day_names_hr_HR, nullptr};
MY_LOCALE my_locale_hr_HR(26, "hr_HR", "Croatian - Croatia", false,
                          &my_locale_typelib_month_names_hr_HR,
                          &my_locale_typelib_ab_month_names_hr_HR,
                          &my_locale_typelib_day_names_hr_HR,
                          &my_locale_typelib_ab_day_names_hr_HR, 8, 11,
                          ',',        /* decimal point hr_HR */
                          '\0',       /* thousands_sep hr_HR */
                          "\x80\x80", /* grouping      hr_HR */
                          &global_errmsgs[en_US]);
/***** LOCALE END hr_HR *****/

/***** LOCALE BEGIN hu_HU: Hungarian - Hungary *****/
static const char *my_locale_month_names_hu_HU[13] = {
    "január",   "február",  "március",   "április",    "május",
    "június",   "július",   "augusztus", "szeptember", "október",
    "november", "december", NullS};
static const char *my_locale_ab_month_names_hu_HU[13] = {
    "jan", "feb", "már", "ápr", "máj", "jún", "júl",
    "aug", "sze", "okt", "nov", "dec", NullS};
static const char *my_locale_day_names_hu_HU[8] = {
    "hétfő",  "kedd",    "szerda",   "csütörtök",
    "péntek", "szombat", "vasárnap", NullS};
static const char *my_locale_ab_day_names_hu_HU[8] = {"h", "k",   "sze", "cs",
                                                      "p", "szo", "v",   NullS};
static TYPELIB my_locale_typelib_month_names_hu_HU = {
    array_elements(my_locale_month_names_hu_HU) - 1, "",
    my_locale_month_names_hu_HU, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_hu_HU = {
    array_elements(my_locale_ab_month_names_hu_HU) - 1, "",
    my_locale_ab_month_names_hu_HU, nullptr};
static TYPELIB my_locale_typelib_day_names_hu_HU = {
    array_elements(my_locale_day_names_hu_HU) - 1, "",
    my_locale_day_names_hu_HU, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_hu_HU = {
    array_elements(my_locale_ab_day_names_hu_HU) - 1, "",
    my_locale_ab_day_names_hu_HU, nullptr};
MY_LOCALE my_locale_hu_HU(27, "hu_HU", "Hungarian - Hungary", false,
                          &my_locale_typelib_month_names_hu_HU,
                          &my_locale_typelib_ab_month_names_hu_HU,
                          &my_locale_typelib_day_names_hu_HU,
                          &my_locale_typelib_ab_day_names_hu_HU, 10, 9,
                          ',',        /* decimal point hu_HU */
                          '.',        /* thousands_sep hu_HU */
                          "\x03\x03", /* grouping      hu_HU */
                          &global_errmsgs[hu_HU]);
/***** LOCALE END hu_HU *****/

/***** LOCALE BEGIN id_ID: Indonesian - Indonesia *****/
static const char *my_locale_month_names_id_ID[13] = {
    "Januari", "Pebruari",  "Maret",   "April",    "Mei",      "Juni", "Juli",
    "Agustus", "September", "Oktober", "November", "Desember", NullS};
static const char *my_locale_ab_month_names_id_ID[13] = {
    "Jan", "Peb", "Mar", "Apr", "Mei", "Jun", "Jul",
    "Agu", "Sep", "Okt", "Nov", "Des", NullS};
static const char *my_locale_day_names_id_ID[8] = {
    "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu", "Minggu", NullS};
static const char *my_locale_ab_day_names_id_ID[8] = {
    "Sen", "Sel", "Rab", "Kam", "Jum", "Sab", "Min", NullS};
static TYPELIB my_locale_typelib_month_names_id_ID = {
    array_elements(my_locale_month_names_id_ID) - 1, "",
    my_locale_month_names_id_ID, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_id_ID = {
    array_elements(my_locale_ab_month_names_id_ID) - 1, "",
    my_locale_ab_month_names_id_ID, nullptr};
static TYPELIB my_locale_typelib_day_names_id_ID = {
    array_elements(my_locale_day_names_id_ID) - 1, "",
    my_locale_day_names_id_ID, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_id_ID = {
    array_elements(my_locale_ab_day_names_id_ID) - 1, "",
    my_locale_ab_day_names_id_ID, nullptr};
MY_LOCALE my_locale_id_ID(28, "id_ID", "Indonesian - Indonesia", true,
                          &my_locale_typelib_month_names_id_ID,
                          &my_locale_typelib_ab_month_names_id_ID,
                          &my_locale_typelib_day_names_id_ID,
                          &my_locale_typelib_ab_day_names_id_ID, 9, 6,
                          ',',        /* decimal point id_ID */
                          '.',        /* thousands_sep id_ID */
                          "\x03\x03", /* grouping      id_ID */
                          &global_errmsgs[en_US]);
/***** LOCALE END id_ID *****/

/***** LOCALE BEGIN is_IS: Icelandic - Iceland *****/
static const char *my_locale_month_names_is_IS[13] = {
    "janúar", "febrúar",   "mars",    "apríl",    "maí",      "júní", "júlí",
    "ágúst",  "september", "október", "nóvember", "desember", NullS};
static const char *my_locale_ab_month_names_is_IS[13] = {
    "jan", "feb", "mar", "apr", "maí", "jún", "júl",
    "ágú", "sep", "okt", "nóv", "des", NullS};
static const char *my_locale_day_names_is_IS[8] = {
    "mánudagur",  "þriðjudagur", "miðvikudagur", "fimmtudagur",
    "föstudagur", "laugardagur", "sunnudagur",   NullS};
static const char *my_locale_ab_day_names_is_IS[8] = {
    "mán", "þri", "mið", "fim", "fös", "lau", "sun", NullS};
static TYPELIB my_locale_typelib_month_names_is_IS = {
    array_elements(my_locale_month_names_is_IS) - 1, "",
    my_locale_month_names_is_IS, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_is_IS = {
    array_elements(my_locale_ab_month_names_is_IS) - 1, "",
    my_locale_ab_month_names_is_IS, nullptr};
static TYPELIB my_locale_typelib_day_names_is_IS = {
    array_elements(my_locale_day_names_is_IS) - 1, "",
    my_locale_day_names_is_IS, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_is_IS = {
    array_elements(my_locale_ab_day_names_is_IS) - 1, "",
    my_locale_ab_day_names_is_IS, nullptr};
MY_LOCALE my_locale_is_IS(29, "is_IS", "Icelandic - Iceland", false,
                          &my_locale_typelib_month_names_is_IS,
                          &my_locale_typelib_ab_month_names_is_IS,
                          &my_locale_typelib_day_names_is_IS,
                          &my_locale_typelib_ab_day_names_is_IS, 9, 12,
                          ',',        /* decimal point is_IS */
                          '.',        /* thousands_sep is_IS */
                          "\x03\x03", /* grouping      is_IS */
                          &global_errmsgs[en_US]);
/***** LOCALE END is_IS *****/

/***** LOCALE BEGIN it_CH: Italian - Switzerland *****/
static const char *my_locale_month_names_it_CH[13] = {
    "gennaio",  "febbraio", "marzo",  "aprile",    "maggio",
    "giugno",   "luglio",   "agosto", "settembre", "ottobre",
    "novembre", "dicembre", NullS};
static const char *my_locale_ab_month_names_it_CH[13] = {
    "gen", "feb", "mar", "apr", "mag", "giu", "lug",
    "ago", "set", "ott", "nov", "dic", NullS};
static const char *my_locale_day_names_it_CH[8] = {
    "lunedì",  "martedì", "mercoledì", "giovedì",
    "venerdì", "sabato",  "domenica",  NullS};
static const char *my_locale_ab_day_names_it_CH[8] = {
    "lun", "mar", "mer", "gio", "ven", "sab", "dom", NullS};
static TYPELIB my_locale_typelib_month_names_it_CH = {
    array_elements(my_locale_month_names_it_CH) - 1, "",
    my_locale_month_names_it_CH, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_it_CH = {
    array_elements(my_locale_ab_month_names_it_CH) - 1, "",
    my_locale_ab_month_names_it_CH, nullptr};
static TYPELIB my_locale_typelib_day_names_it_CH = {
    array_elements(my_locale_day_names_it_CH) - 1, "",
    my_locale_day_names_it_CH, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_it_CH = {
    array_elements(my_locale_ab_day_names_it_CH) - 1, "",
    my_locale_ab_day_names_it_CH, nullptr};
MY_LOCALE my_locale_it_CH(30, "it_CH", "Italian - Switzerland", false,
                          &my_locale_typelib_month_names_it_CH,
                          &my_locale_typelib_ab_month_names_it_CH,
                          &my_locale_typelib_day_names_it_CH,
                          &my_locale_typelib_ab_day_names_it_CH, 9, 9,
                          ',',        /* decimal point it_CH */
                          '\'',       /* thousands_sep it_CH */
                          "\x03\x03", /* grouping      it_CH */
                          &global_errmsgs[it_IT]);
/***** LOCALE END it_CH *****/

/***** LOCALE BEGIN ja_JP: Japanese - Japan *****/
static const char *my_locale_month_names_ja_JP[13] = {
    "1月", "2月", "3月",  "4月",  "5月",  "6月", "7月",
    "8月", "9月", "10月", "11月", "12月", NullS};
static const char *my_locale_ab_month_names_ja_JP[13] = {
    " 1月", " 2月", " 3月", " 4月", " 5月", " 6月", " 7月",
    " 8月", " 9月", "10月", "11月", "12月", NullS};
static const char *my_locale_day_names_ja_JP[8] = {"月曜日", "火曜日", "水曜日",
                                                   "木曜日", "金曜日", "土曜日",
                                                   "日曜日", NullS};
static const char *my_locale_ab_day_names_ja_JP[8] = {"月", "火", "水", "木",
                                                      "金", "土", "日", NullS};
static TYPELIB my_locale_typelib_month_names_ja_JP = {
    array_elements(my_locale_month_names_ja_JP) - 1, "",
    my_locale_month_names_ja_JP, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ja_JP = {
    array_elements(my_locale_ab_month_names_ja_JP) - 1, "",
    my_locale_ab_month_names_ja_JP, nullptr};
static TYPELIB my_locale_typelib_day_names_ja_JP = {
    array_elements(my_locale_day_names_ja_JP) - 1, "",
    my_locale_day_names_ja_JP, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ja_JP = {
    array_elements(my_locale_ab_day_names_ja_JP) - 1, "",
    my_locale_ab_day_names_ja_JP, nullptr};
MY_LOCALE my_locale_ja_JP(2, "ja_JP", "Japanese - Japan", false,
                          &my_locale_typelib_month_names_ja_JP,
                          &my_locale_typelib_ab_month_names_ja_JP,
                          &my_locale_typelib_day_names_ja_JP,
                          &my_locale_typelib_ab_day_names_ja_JP, 3, 3,
                          '.',    /* decimal point ja_JP */
                          ',',    /* thousands_sep ja_JP */
                          "\x03", /* grouping      ja_JP */
                          &global_errmsgs[ja_JP]);
/***** LOCALE END ja_JP *****/

/***** LOCALE BEGIN ko_KR: Korean - Korea *****/
static const char *my_locale_month_names_ko_KR[13] = {
    "일월", "이월", "삼월", "사월",   "오월",   "유월", "칠월",
    "팔월", "구월", "시월", "십일월", "십이월", NullS};
static const char *my_locale_ab_month_names_ko_KR[13] = {
    " 1월", " 2월", " 3월", " 4월", " 5월", " 6월", " 7월",
    " 8월", " 9월", "10월", "11월", "12월", NullS};
static const char *my_locale_day_names_ko_KR[8] = {"월요일", "화요일", "수요일",
                                                   "목요일", "금요일", "토요일",
                                                   "일요일", NullS};
static const char *my_locale_ab_day_names_ko_KR[8] = {"월", "화", "수", "목",
                                                      "금", "토", "일", NullS};
static TYPELIB my_locale_typelib_month_names_ko_KR = {
    array_elements(my_locale_month_names_ko_KR) - 1, "",
    my_locale_month_names_ko_KR, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ko_KR = {
    array_elements(my_locale_ab_month_names_ko_KR) - 1, "",
    my_locale_ab_month_names_ko_KR, nullptr};
static TYPELIB my_locale_typelib_day_names_ko_KR = {
    array_elements(my_locale_day_names_ko_KR) - 1, "",
    my_locale_day_names_ko_KR, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ko_KR = {
    array_elements(my_locale_ab_day_names_ko_KR) - 1, "",
    my_locale_ab_day_names_ko_KR, nullptr};
MY_LOCALE my_locale_ko_KR(31, "ko_KR", "Korean - Korea", false,
                          &my_locale_typelib_month_names_ko_KR,
                          &my_locale_typelib_ab_month_names_ko_KR,
                          &my_locale_typelib_day_names_ko_KR,
                          &my_locale_typelib_ab_day_names_ko_KR, 3, 3,
                          '.',        /* decimal point ko_KR */
                          ',',        /* thousands_sep ko_KR */
                          "\x03\x03", /* grouping      ko_KR */
                          &global_errmsgs[ko_KR]);
/***** LOCALE END ko_KR *****/

/***** LOCALE BEGIN lt_LT: Lithuanian - Lithuania *****/
static const char *my_locale_month_names_lt_LT[13] = {
    "sausio",    "vasario",  "kovo",      "balandžio", "gegužės",
    "birželio",  "liepos",   "rugpjūčio", "rugsėjo",   "spalio",
    "lapkričio", "gruodžio", NullS};
static const char *my_locale_ab_month_names_lt_LT[13] = {
    "Sau", "Vas", "Kov", "Bal", "Geg", "Bir", "Lie",
    "Rgp", "Rgs", "Spa", "Lap", "Grd", NullS};
static const char *my_locale_day_names_lt_LT[8] = {
    "Pirmadienis",  "Antradienis", "Trečiadienis", "Ketvirtadienis",
    "Penktadienis", "Šeštadienis", "Sekmadienis",  NullS};
static const char *my_locale_ab_day_names_lt_LT[8] = {"Pr", "An", "Tr", "Kt",
                                                      "Pn", "Št", "Sk", NullS};
static TYPELIB my_locale_typelib_month_names_lt_LT = {
    array_elements(my_locale_month_names_lt_LT) - 1, "",
    my_locale_month_names_lt_LT, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_lt_LT = {
    array_elements(my_locale_ab_month_names_lt_LT) - 1, "",
    my_locale_ab_month_names_lt_LT, nullptr};
static TYPELIB my_locale_typelib_day_names_lt_LT = {
    array_elements(my_locale_day_names_lt_LT) - 1, "",
    my_locale_day_names_lt_LT, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_lt_LT = {
    array_elements(my_locale_ab_day_names_lt_LT) - 1, "",
    my_locale_ab_day_names_lt_LT, nullptr};
MY_LOCALE my_locale_lt_LT(32, "lt_LT", "Lithuanian - Lithuania", false,
                          &my_locale_typelib_month_names_lt_LT,
                          &my_locale_typelib_ab_month_names_lt_LT,
                          &my_locale_typelib_day_names_lt_LT,
                          &my_locale_typelib_ab_day_names_lt_LT, 9, 14,
                          ',',        /* decimal point lt_LT */
                          '.',        /* thousands_sep lt_LT */
                          "\x03\x03", /* grouping      lt_LT */
                          &global_errmsgs[en_US]);
/***** LOCALE END lt_LT *****/

/***** LOCALE BEGIN lv_LV: Latvian - Latvia *****/
static const char *my_locale_month_names_lv_LV[13] = {
    "janvāris",  "februāris", "marts",   "aprīlis",    "maijs",
    "jūnijs",    "jūlijs",    "augusts", "septembris", "oktobris",
    "novembris", "decembris", NullS};
static const char *my_locale_ab_month_names_lv_LV[13] = {
    "jan", "feb", "mar", "apr", "mai", "jūn", "jūl",
    "aug", "sep", "okt", "nov", "dec", NullS};
static const char *my_locale_day_names_lv_LV[8] = {
    "pirmdiena",  "otrdiena",  "trešdiena", "ceturtdiena",
    "piektdiena", "sestdiena", "svētdiena", NullS};
static const char *my_locale_ab_day_names_lv_LV[8] = {"P ", "O ", "T ", "C ",
                                                      "Pk", "S ", "Sv", NullS};
static TYPELIB my_locale_typelib_month_names_lv_LV = {
    array_elements(my_locale_month_names_lv_LV) - 1, "",
    my_locale_month_names_lv_LV, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_lv_LV = {
    array_elements(my_locale_ab_month_names_lv_LV) - 1, "",
    my_locale_ab_month_names_lv_LV, nullptr};
static TYPELIB my_locale_typelib_day_names_lv_LV = {
    array_elements(my_locale_day_names_lv_LV) - 1, "",
    my_locale_day_names_lv_LV, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_lv_LV = {
    array_elements(my_locale_ab_day_names_lv_LV) - 1, "",
    my_locale_ab_day_names_lv_LV, nullptr};
MY_LOCALE my_locale_lv_LV(33, "lv_LV", "Latvian - Latvia", false,
                          &my_locale_typelib_month_names_lv_LV,
                          &my_locale_typelib_ab_month_names_lv_LV,
                          &my_locale_typelib_day_names_lv_LV,
                          &my_locale_typelib_ab_day_names_lv_LV, 10, 11,
                          ',',        /* decimal point lv_LV */
                          ' ',        /* thousands_sep lv_LV */
                          "\x03\x03", /* grouping      lv_LV */
                          &global_errmsgs[en_US]);
/***** LOCALE END lv_LV *****/

/***** LOCALE BEGIN mk_MK: Macedonian - FYROM *****/
static const char *my_locale_month_names_mk_MK[13] = {
    "јануари", "февруари",  "март",     "април",   "мај",      "јуни", "јули",
    "август",  "септември", "октомври", "ноември", "декември", NullS};
static const char *my_locale_ab_month_names_mk_MK[13] = {
    "јан", "фев", "мар", "апр", "мај", "јун", "јул",
    "авг", "сеп", "окт", "ное", "дек", NullS};
static const char *my_locale_day_names_mk_MK[8] = {
    "понеделник", "вторник", "среда",  "четврток",
    "петок",      "сабота",  "недела", NullS};
static const char *my_locale_ab_day_names_mk_MK[8] = {
    "пон", "вто", "сре", "чет", "пет", "саб", "нед", NullS};
static TYPELIB my_locale_typelib_month_names_mk_MK = {
    array_elements(my_locale_month_names_mk_MK) - 1, "",
    my_locale_month_names_mk_MK, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_mk_MK = {
    array_elements(my_locale_ab_month_names_mk_MK) - 1, "",
    my_locale_ab_month_names_mk_MK, nullptr};
static TYPELIB my_locale_typelib_day_names_mk_MK = {
    array_elements(my_locale_day_names_mk_MK) - 1, "",
    my_locale_day_names_mk_MK, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_mk_MK = {
    array_elements(my_locale_ab_day_names_mk_MK) - 1, "",
    my_locale_ab_day_names_mk_MK, nullptr};
MY_LOCALE my_locale_mk_MK(34, "mk_MK", "Macedonian - FYROM", false,
                          &my_locale_typelib_month_names_mk_MK,
                          &my_locale_typelib_ab_month_names_mk_MK,
                          &my_locale_typelib_day_names_mk_MK,
                          &my_locale_typelib_ab_day_names_mk_MK, 9, 10,
                          ',',        /* decimal point mk_MK */
                          ' ',        /* thousands_sep mk_MK */
                          "\x03\x03", /* grouping      mk_MK */
                          &global_errmsgs[en_US]);
/***** LOCALE END mk_MK *****/

/***** LOCALE BEGIN mn_MN: Mongolia - Mongolian *****/
static const char *my_locale_month_names_mn_MN[13] = {"Нэгдүгээр сар",
                                                      "Хоёрдугаар сар",
                                                      "Гуравдугаар сар",
                                                      "Дөрөвдүгээр сар",
                                                      "Тавдугаар сар",
                                                      "Зургаадугар сар",
                                                      "Долоодугаар сар",
                                                      "Наймдугаар сар",
                                                      "Есдүгээр сар",
                                                      "Аравдугаар сар",
                                                      "Арваннэгдүгээр сар",
                                                      "Арванхоёрдгаар сар",
                                                      NullS};
static const char *my_locale_ab_month_names_mn_MN[13] = {
    "1-р", "2-р", "3-р",  "4-р",  "5-р",  "6-р", "7-р",
    "8-р", "9-р", "10-р", "11-р", "12-р", NullS};
static const char *my_locale_day_names_mn_MN[8] = {
    "Даваа", "Мягмар", "Лхагва", "Пүрэв", "Баасан", "Бямба", "Ням", NullS};
static const char *my_locale_ab_day_names_mn_MN[8] = {"Да", "Мя", "Лх", "Пү",
                                                      "Ба", "Бя", "Ня", NullS};
static TYPELIB my_locale_typelib_month_names_mn_MN = {
    array_elements(my_locale_month_names_mn_MN) - 1, "",
    my_locale_month_names_mn_MN, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_mn_MN = {
    array_elements(my_locale_ab_month_names_mn_MN) - 1, "",
    my_locale_ab_month_names_mn_MN, nullptr};
static TYPELIB my_locale_typelib_day_names_mn_MN = {
    array_elements(my_locale_day_names_mn_MN) - 1, "",
    my_locale_day_names_mn_MN, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_mn_MN = {
    array_elements(my_locale_ab_day_names_mn_MN) - 1, "",
    my_locale_ab_day_names_mn_MN, nullptr};
MY_LOCALE my_locale_mn_MN(35, "mn_MN", "Mongolia - Mongolian", false,
                          &my_locale_typelib_month_names_mn_MN,
                          &my_locale_typelib_ab_month_names_mn_MN,
                          &my_locale_typelib_day_names_mn_MN,
                          &my_locale_typelib_ab_day_names_mn_MN, 18, 6,
                          ',',        /* decimal point mn_MN */
                          '.',        /* thousands_sep mn_MN */
                          "\x03\x03", /* grouping      mn_MN */
                          &global_errmsgs[en_US]);
/***** LOCALE END mn_MN *****/

/***** LOCALE BEGIN ms_MY: Malay - Malaysia *****/
static const char *my_locale_month_names_ms_MY[13] = {
    "Januari", "Februari",  "Mac",     "April",    "Mei",      "Jun", "Julai",
    "Ogos",    "September", "Oktober", "November", "Disember", NullS};
static const char *my_locale_ab_month_names_ms_MY[13] = {
    "Jan",  "Feb", "Mac", "Apr", "Mei", "Jun", "Jul",
    "Ogos", "Sep", "Okt", "Nov", "Dis", NullS};
static const char *my_locale_day_names_ms_MY[8] = {
    "Isnin", "Selasa", "Rabu", "Khamis", "Jumaat", "Sabtu", "Ahad", NullS};
static const char *my_locale_ab_day_names_ms_MY[8] = {
    "Isn", "Sel", "Rab", "Kha", "Jum", "Sab", "Ahd", NullS};
static TYPELIB my_locale_typelib_month_names_ms_MY = {
    array_elements(my_locale_month_names_ms_MY) - 1, "",
    my_locale_month_names_ms_MY, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ms_MY = {
    array_elements(my_locale_ab_month_names_ms_MY) - 1, "",
    my_locale_ab_month_names_ms_MY, nullptr};
static TYPELIB my_locale_typelib_day_names_ms_MY = {
    array_elements(my_locale_day_names_ms_MY) - 1, "",
    my_locale_day_names_ms_MY, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ms_MY = {
    array_elements(my_locale_ab_day_names_ms_MY) - 1, "",
    my_locale_ab_day_names_ms_MY, nullptr};
MY_LOCALE my_locale_ms_MY(36, "ms_MY", "Malay - Malaysia", true,
                          &my_locale_typelib_month_names_ms_MY,
                          &my_locale_typelib_ab_month_names_ms_MY,
                          &my_locale_typelib_day_names_ms_MY,
                          &my_locale_typelib_ab_day_names_ms_MY, 9, 6,
                          '.',    /* decimal point ms_MY */
                          ',',    /* thousands_sep ms_MY */
                          "\x03", /* grouping      ms_MY */
                          &global_errmsgs[en_US]);
/***** LOCALE END ms_MY *****/

/***** LOCALE BEGIN nb_NO: Norwegian(Bokml) - Norway *****/
static const char *my_locale_month_names_nb_NO[13] = {
    "januar", "februar",   "mars",    "april",    "mai",      "juni", "juli",
    "august", "september", "oktober", "november", "desember", NullS};
static const char *my_locale_ab_month_names_nb_NO[13] = {
    "jan", "feb", "mar", "apr", "mai", "jun", "jul",
    "aug", "sep", "okt", "nov", "des", NullS};
static const char *my_locale_day_names_nb_NO[8] = {
    "mandag", "tirsdag", "onsdag", "torsdag",
    "fredag", "lørdag",  "søndag", NullS};
static const char *my_locale_ab_day_names_nb_NO[8] = {
    "man", "tir", "ons", "tor", "fre", "lør", "søn", NullS};
static TYPELIB my_locale_typelib_month_names_nb_NO = {
    array_elements(my_locale_month_names_nb_NO) - 1, "",
    my_locale_month_names_nb_NO, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_nb_NO = {
    array_elements(my_locale_ab_month_names_nb_NO) - 1, "",
    my_locale_ab_month_names_nb_NO, nullptr};
static TYPELIB my_locale_typelib_day_names_nb_NO = {
    array_elements(my_locale_day_names_nb_NO) - 1, "",
    my_locale_day_names_nb_NO, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_nb_NO = {
    array_elements(my_locale_ab_day_names_nb_NO) - 1, "",
    my_locale_ab_day_names_nb_NO, nullptr};
MY_LOCALE my_locale_nb_NO(37, "nb_NO", "Norwegian(Bokml) - Norway", false,
                          &my_locale_typelib_month_names_nb_NO,
                          &my_locale_typelib_ab_month_names_nb_NO,
                          &my_locale_typelib_day_names_nb_NO,
                          &my_locale_typelib_ab_day_names_nb_NO, 9, 7,
                          ',',        /* decimal point nb_NO */
                          '.',        /* thousands_sep nb_NO */
                          "\x03\x03", /* grouping      nb_NO */
                          &global_errmsgs[no_NO]);
/***** LOCALE END nb_NO *****/

/***** LOCALE BEGIN nl_NL: Dutch - The Netherlands *****/
static const char *my_locale_month_names_nl_NL[13] = {
    "januari",  "februari",  "maart",   "april",    "mei",      "juni", "juli",
    "augustus", "september", "oktober", "november", "december", NullS};
static const char *my_locale_ab_month_names_nl_NL[13] = {
    "jan", "feb", "mrt", "apr", "mei", "jun", "jul",
    "aug", "sep", "okt", "nov", "dec", NullS};
static const char *my_locale_day_names_nl_NL[8] = {
    "maandag", "dinsdag",  "woensdag", "donderdag",
    "vrijdag", "zaterdag", "zondag",   NullS};
static const char *my_locale_ab_day_names_nl_NL[8] = {"ma", "di", "wo", "do",
                                                      "vr", "za", "zo", NullS};
static TYPELIB my_locale_typelib_month_names_nl_NL = {
    array_elements(my_locale_month_names_nl_NL) - 1, "",
    my_locale_month_names_nl_NL, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_nl_NL = {
    array_elements(my_locale_ab_month_names_nl_NL) - 1, "",
    my_locale_ab_month_names_nl_NL, nullptr};
static TYPELIB my_locale_typelib_day_names_nl_NL = {
    array_elements(my_locale_day_names_nl_NL) - 1, "",
    my_locale_day_names_nl_NL, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_nl_NL = {
    array_elements(my_locale_ab_day_names_nl_NL) - 1, "",
    my_locale_ab_day_names_nl_NL, nullptr};
MY_LOCALE my_locale_nl_NL(38, "nl_NL", "Dutch - The Netherlands", true,
                          &my_locale_typelib_month_names_nl_NL,
                          &my_locale_typelib_ab_month_names_nl_NL,
                          &my_locale_typelib_day_names_nl_NL,
                          &my_locale_typelib_ab_day_names_nl_NL, 9, 9,
                          ',',        /* decimal point nl_NL */
                          '\0',       /* thousands_sep nl_NL */
                          "\x80\x80", /* grouping      nl_NL */
                          &global_errmsgs[nl_NL]);
/***** LOCALE END nl_NL *****/

/***** LOCALE BEGIN pl_PL: Polish - Poland *****/
static const char *my_locale_month_names_pl_PL[13] = {
    "styczeń",  "luty",     "marzec",   "kwiecień", "maj",
    "czerwiec", "lipiec",   "sierpień", "wrzesień", "październik",
    "listopad", "grudzień", NullS};
static const char *my_locale_ab_month_names_pl_PL[13] = {
    "sty", "lut", "mar", "kwi", "maj", "cze", "lip",
    "sie", "wrz", "paź", "lis", "gru", NullS};
static const char *my_locale_day_names_pl_PL[8] = {
    "poniedziałek", "wtorek", "środa",     "czwartek",
    "piątek",       "sobota", "niedziela", NullS};
static const char *my_locale_ab_day_names_pl_PL[8] = {
    "pon", "wto", "śro", "czw", "pią", "sob", "nie", NullS};
static TYPELIB my_locale_typelib_month_names_pl_PL = {
    array_elements(my_locale_month_names_pl_PL) - 1, "",
    my_locale_month_names_pl_PL, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_pl_PL = {
    array_elements(my_locale_ab_month_names_pl_PL) - 1, "",
    my_locale_ab_month_names_pl_PL, nullptr};
static TYPELIB my_locale_typelib_day_names_pl_PL = {
    array_elements(my_locale_day_names_pl_PL) - 1, "",
    my_locale_day_names_pl_PL, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_pl_PL = {
    array_elements(my_locale_ab_day_names_pl_PL) - 1, "",
    my_locale_ab_day_names_pl_PL, nullptr};
MY_LOCALE my_locale_pl_PL(39, "pl_PL", "Polish - Poland", false,
                          &my_locale_typelib_month_names_pl_PL,
                          &my_locale_typelib_ab_month_names_pl_PL,
                          &my_locale_typelib_day_names_pl_PL,
                          &my_locale_typelib_ab_day_names_pl_PL, 11, 12,
                          ',',        /* decimal point pl_PL */
                          '\0',       /* thousands_sep pl_PL */
                          "\x80\x80", /* grouping      pl_PL */
                          &global_errmsgs[pl_PL]);
/***** LOCALE END pl_PL *****/

/***** LOCALE BEGIN pt_BR: Portugese - Brazil *****/
static const char *my_locale_month_names_pt_BR[13] = {
    "janeiro", "fevereiro", "março",   "abril",    "maio",     "junho", "julho",
    "agosto",  "setembro",  "outubro", "novembro", "dezembro", NullS};
static const char *my_locale_ab_month_names_pt_BR[13] = {
    "Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul",
    "Ago", "Set", "Out", "Nov", "Dez", NullS};
static const char *my_locale_day_names_pt_BR[8] = {"segunda", "terça", "quarta",
                                                   "quinta",  "sexta", "sábado",
                                                   "domingo", NullS};
static const char *my_locale_ab_day_names_pt_BR[8] = {
    "Seg", "Ter", "Qua", "Qui", "Sex", "Sáb", "Dom", NullS};
static TYPELIB my_locale_typelib_month_names_pt_BR = {
    array_elements(my_locale_month_names_pt_BR) - 1, "",
    my_locale_month_names_pt_BR, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_pt_BR = {
    array_elements(my_locale_ab_month_names_pt_BR) - 1, "",
    my_locale_ab_month_names_pt_BR, nullptr};
static TYPELIB my_locale_typelib_day_names_pt_BR = {
    array_elements(my_locale_day_names_pt_BR) - 1, "",
    my_locale_day_names_pt_BR, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_pt_BR = {
    array_elements(my_locale_ab_day_names_pt_BR) - 1, "",
    my_locale_ab_day_names_pt_BR, nullptr};
MY_LOCALE my_locale_pt_BR(40, "pt_BR", "Portugese - Brazil", false,
                          &my_locale_typelib_month_names_pt_BR,
                          &my_locale_typelib_ab_month_names_pt_BR,
                          &my_locale_typelib_day_names_pt_BR,
                          &my_locale_typelib_ab_day_names_pt_BR, 9, 7,
                          ',',        /* decimal point pt_BR */
                          '\0',       /* thousands_sep pt_BR */
                          "\x80\x80", /* grouping      pt_BR */
                          &global_errmsgs[pt_PT]);
/***** LOCALE END pt_BR *****/

/***** LOCALE BEGIN pt_PT: Portugese - Portugal *****/
static const char *my_locale_month_names_pt_PT[13] = {
    "Janeiro", "Fevereiro", "Março",   "Abril",    "Maio",     "Junho", "Julho",
    "Agosto",  "Setembro",  "Outubro", "Novembro", "Dezembro", NullS};
static const char *my_locale_ab_month_names_pt_PT[13] = {
    "Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul",
    "Ago", "Set", "Out", "Nov", "Dez", NullS};
static const char *my_locale_day_names_pt_PT[8] = {"Segunda", "Terça", "Quarta",
                                                   "Quinta",  "Sexta", "Sábado",
                                                   "Domingo", NullS};
static const char *my_locale_ab_day_names_pt_PT[8] = {
    "Seg", "Ter", "Qua", "Qui", "Sex", "Sáb", "Dom", NullS};
static TYPELIB my_locale_typelib_month_names_pt_PT = {
    array_elements(my_locale_month_names_pt_PT) - 1, "",
    my_locale_month_names_pt_PT, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_pt_PT = {
    array_elements(my_locale_ab_month_names_pt_PT) - 1, "",
    my_locale_ab_month_names_pt_PT, nullptr};
static TYPELIB my_locale_typelib_day_names_pt_PT = {
    array_elements(my_locale_day_names_pt_PT) - 1, "",
    my_locale_day_names_pt_PT, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_pt_PT = {
    array_elements(my_locale_ab_day_names_pt_PT) - 1, "",
    my_locale_ab_day_names_pt_PT, nullptr};
MY_LOCALE my_locale_pt_PT(41, "pt_PT", "Portugese - Portugal", false,
                          &my_locale_typelib_month_names_pt_PT,
                          &my_locale_typelib_ab_month_names_pt_PT,
                          &my_locale_typelib_day_names_pt_PT,
                          &my_locale_typelib_ab_day_names_pt_PT, 9, 7,
                          ',',        /* decimal point pt_PT */
                          '\0',       /* thousands_sep pt_PT */
                          "\x80\x80", /* grouping      pt_PT */
                          &global_errmsgs[pt_PT]);
/***** LOCALE END pt_PT *****/

/***** LOCALE BEGIN ro_RO: Romanian - Romania *****/
static const char *my_locale_month_names_ro_RO[13] = {
    "Ianuarie",  "Februarie", "Martie", "Aprilie",    "Mai",
    "Iunie",     "Iulie",     "August", "Septembrie", "Octombrie",
    "Noiembrie", "Decembrie", NullS};
static const char *my_locale_ab_month_names_ro_RO[13] = {
    "ian", "feb", "mar", "apr", "mai", "iun", "iul",
    "aug", "sep", "oct", "nov", "dec", NullS};
static const char *my_locale_day_names_ro_RO[8] = {
    "Luni", "Marţi", "Miercuri", "Joi", "Vineri", "Sâmbătă", "Duminică", NullS};
static const char *my_locale_ab_day_names_ro_RO[8] = {"Lu", "Ma", "Mi", "Jo",
                                                      "Vi", "Sâ", "Du", NullS};
static TYPELIB my_locale_typelib_month_names_ro_RO = {
    array_elements(my_locale_month_names_ro_RO) - 1, "",
    my_locale_month_names_ro_RO, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ro_RO = {
    array_elements(my_locale_ab_month_names_ro_RO) - 1, "",
    my_locale_ab_month_names_ro_RO, nullptr};
static TYPELIB my_locale_typelib_day_names_ro_RO = {
    array_elements(my_locale_day_names_ro_RO) - 1, "",
    my_locale_day_names_ro_RO, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ro_RO = {
    array_elements(my_locale_ab_day_names_ro_RO) - 1, "",
    my_locale_ab_day_names_ro_RO, nullptr};
MY_LOCALE my_locale_ro_RO(42, "ro_RO", "Romanian - Romania", false,
                          &my_locale_typelib_month_names_ro_RO,
                          &my_locale_typelib_ab_month_names_ro_RO,
                          &my_locale_typelib_day_names_ro_RO,
                          &my_locale_typelib_ab_day_names_ro_RO, 10, 8,
                          ',',        /* decimal point ro_RO */
                          '.',        /* thousands_sep ro_RO */
                          "\x03\x03", /* grouping      ro_RO */
                          &global_errmsgs[ro_RO]);
/***** LOCALE END ro_RO *****/

/***** LOCALE BEGIN ru_RU: Russian - Russia *****/
static const char *my_locale_month_names_ru_RU[13] = {
    "Января",  "Февраля",  "Марта",   "Апреля", "Мая",     "Июня", "Июля",
    "Августа", "Сентября", "Октября", "Ноября", "Декабря", NullS};
static const char *my_locale_ab_month_names_ru_RU[13] = {
    "Янв", "Фев", "Мар", "Апр", "Май", "Июн", "Июл",
    "Авг", "Сен", "Окт", "Ноя", "Дек", NullS};
static const char *my_locale_day_names_ru_RU[8] = {
    "Понедельник", "Вторник", "Среда",       "Четверг",
    "Пятница",     "Суббота", "Воскресенье", NullS};
static const char *my_locale_ab_day_names_ru_RU[8] = {
    "Пнд", "Втр", "Срд", "Чтв", "Птн", "Сбт", "Вск", NullS};
static TYPELIB my_locale_typelib_month_names_ru_RU = {
    array_elements(my_locale_month_names_ru_RU) - 1, "",
    my_locale_month_names_ru_RU, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ru_RU = {
    array_elements(my_locale_ab_month_names_ru_RU) - 1, "",
    my_locale_ab_month_names_ru_RU, nullptr};
static TYPELIB my_locale_typelib_day_names_ru_RU = {
    array_elements(my_locale_day_names_ru_RU) - 1, "",
    my_locale_day_names_ru_RU, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ru_RU = {
    array_elements(my_locale_ab_day_names_ru_RU) - 1, "",
    my_locale_ab_day_names_ru_RU, nullptr};
MY_LOCALE my_locale_ru_RU(43, "ru_RU", "Russian - Russia", false,
                          &my_locale_typelib_month_names_ru_RU,
                          &my_locale_typelib_ab_month_names_ru_RU,
                          &my_locale_typelib_day_names_ru_RU,
                          &my_locale_typelib_ab_day_names_ru_RU, 8, 11,
                          ',',        /* decimal point ru_RU */
                          ' ',        /* thousands_sep ru_RU */
                          "\x03\x03", /* grouping      ru_RU */
                          &global_errmsgs[ru_RU]);
/***** LOCALE END ru_RU *****/

/***** LOCALE BEGIN ru_UA: Russian - Ukraine *****/
static const char *my_locale_month_names_ru_UA[13] = {
    "Январь", "Февраль",  "Март",    "Апрель", "Май",     "Июнь", "Июль",
    "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь", NullS};
static const char *my_locale_ab_month_names_ru_UA[13] = {
    "Янв", "Фев", "Мар", "Апр", "Май", "Июн", "Июл",
    "Авг", "Сен", "Окт", "Ноя", "Дек", NullS};
static const char *my_locale_day_names_ru_UA[8] = {
    "Понедельник", "Вторник", "Среда",       "Четверг",
    "Пятница",     "Суббота", "Воскресенье", NullS};
static const char *my_locale_ab_day_names_ru_UA[8] = {
    "Пнд", "Вто", "Срд", "Чтв", "Птн", "Суб", "Вск", NullS};
static TYPELIB my_locale_typelib_month_names_ru_UA = {
    array_elements(my_locale_month_names_ru_UA) - 1, "",
    my_locale_month_names_ru_UA, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ru_UA = {
    array_elements(my_locale_ab_month_names_ru_UA) - 1, "",
    my_locale_ab_month_names_ru_UA, nullptr};
static TYPELIB my_locale_typelib_day_names_ru_UA = {
    array_elements(my_locale_day_names_ru_UA) - 1, "",
    my_locale_day_names_ru_UA, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ru_UA = {
    array_elements(my_locale_ab_day_names_ru_UA) - 1, "",
    my_locale_ab_day_names_ru_UA, nullptr};
MY_LOCALE my_locale_ru_UA(44, "ru_UA", "Russian - Ukraine", false,
                          &my_locale_typelib_month_names_ru_UA,
                          &my_locale_typelib_ab_month_names_ru_UA,
                          &my_locale_typelib_day_names_ru_UA,
                          &my_locale_typelib_ab_day_names_ru_UA, 8, 11,
                          ',',        /* decimal point ru_UA */
                          '.',        /* thousands_sep ru_UA */
                          "\x03\x03", /* grouping      ru_UA */
                          &global_errmsgs[ru_RU]);
/***** LOCALE END ru_UA *****/

/***** LOCALE BEGIN sk_SK: Slovak - Slovakia *****/
static const char *my_locale_month_names_sk_SK[13] = {
    "január", "február",   "marec",   "apríl",    "máj",      "jún", "júl",
    "august", "september", "október", "november", "december", NullS};
static const char *my_locale_ab_month_names_sk_SK[13] = {
    "jan", "feb", "mar", "apr", "máj", "jún", "júl",
    "aug", "sep", "okt", "nov", "dec", NullS};
static const char *my_locale_day_names_sk_SK[8] = {
    "Pondelok", "Utorok", "Streda", "Štvrtok",
    "Piatok",   "Sobota", "Nedeľa", NullS};
static const char *my_locale_ab_day_names_sk_SK[8] = {"Po", "Ut", "St", "Št",
                                                      "Pi", "So", "Ne", NullS};
static TYPELIB my_locale_typelib_month_names_sk_SK = {
    array_elements(my_locale_month_names_sk_SK) - 1, "",
    my_locale_month_names_sk_SK, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_sk_SK = {
    array_elements(my_locale_ab_month_names_sk_SK) - 1, "",
    my_locale_ab_month_names_sk_SK, nullptr};
static TYPELIB my_locale_typelib_day_names_sk_SK = {
    array_elements(my_locale_day_names_sk_SK) - 1, "",
    my_locale_day_names_sk_SK, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_sk_SK = {
    array_elements(my_locale_ab_day_names_sk_SK) - 1, "",
    my_locale_ab_day_names_sk_SK, nullptr};
MY_LOCALE my_locale_sk_SK(45, "sk_SK", "Slovak - Slovakia", false,
                          &my_locale_typelib_month_names_sk_SK,
                          &my_locale_typelib_ab_month_names_sk_SK,
                          &my_locale_typelib_day_names_sk_SK,
                          &my_locale_typelib_ab_day_names_sk_SK, 9, 8,
                          ',',        /* decimal point sk_SK */
                          ' ',        /* thousands_sep sk_SK */
                          "\x03\x03", /* grouping      sk_SK */
                          &global_errmsgs[sk_SK]);
/***** LOCALE END sk_SK *****/

/***** LOCALE BEGIN sl_SI: Slovenian - Slovenia *****/
static const char *my_locale_month_names_sl_SI[13] = {
    "januar", "februar",   "marec",   "april",    "maj",      "junij", "julij",
    "avgust", "september", "oktober", "november", "december", NullS};
static const char *my_locale_ab_month_names_sl_SI[13] = {
    "jan", "feb", "mar", "apr", "maj", "jun", "jul",
    "avg", "sep", "okt", "nov", "dec", NullS};
static const char *my_locale_day_names_sl_SI[8] = {
    "ponedeljek", "torek",  "sreda",   "četrtek",
    "petek",      "sobota", "nedelja", NullS};
static const char *my_locale_ab_day_names_sl_SI[8] = {
    "pon", "tor", "sre", "čet", "pet", "sob", "ned", NullS};
static TYPELIB my_locale_typelib_month_names_sl_SI = {
    array_elements(my_locale_month_names_sl_SI) - 1, "",
    my_locale_month_names_sl_SI, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_sl_SI = {
    array_elements(my_locale_ab_month_names_sl_SI) - 1, "",
    my_locale_ab_month_names_sl_SI, nullptr};
static TYPELIB my_locale_typelib_day_names_sl_SI = {
    array_elements(my_locale_day_names_sl_SI) - 1, "",
    my_locale_day_names_sl_SI, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_sl_SI = {
    array_elements(my_locale_ab_day_names_sl_SI) - 1, "",
    my_locale_ab_day_names_sl_SI, nullptr};
MY_LOCALE my_locale_sl_SI(46, "sl_SI", "Slovenian - Slovenia", false,
                          &my_locale_typelib_month_names_sl_SI,
                          &my_locale_typelib_ab_month_names_sl_SI,
                          &my_locale_typelib_day_names_sl_SI,
                          &my_locale_typelib_ab_day_names_sl_SI, 9, 10,
                          ',',        /* decimal point sl_SI */
                          ' ',        /* thousands_sep sl_SI */
                          "\x80\x80", /* grouping      sl_SI */
                          &global_errmsgs[en_US]);
/***** LOCALE END sl_SI *****/

/***** LOCALE BEGIN sq_AL: Albanian - Albania *****/
static const char *my_locale_month_names_sq_AL[13] = {
    "janar", "shkurt",  "mars",  "prill",  "maj",     "qershor", "korrik",
    "gusht", "shtator", "tetor", "nëntor", "dhjetor", NullS};
static const char *my_locale_ab_month_names_sq_AL[13] = {
    "Jan", "Shk", "Mar", "Pri", "Maj", "Qer", "Kor",
    "Gsh", "Sht", "Tet", "Nën", "Dhj", NullS};
static const char *my_locale_day_names_sq_AL[8] = {
    "e hënë ",   "e martë ",  "e mërkurë ", "e enjte ",
    "e premte ", "e shtunë ", "e diel ",    NullS};
static const char *my_locale_ab_day_names_sq_AL[8] = {
    "Hën ", "Mar ", "Mër ", "Enj ", "Pre ", "Sht ", "Die ", NullS};
static TYPELIB my_locale_typelib_month_names_sq_AL = {
    array_elements(my_locale_month_names_sq_AL) - 1, "",
    my_locale_month_names_sq_AL, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_sq_AL = {
    array_elements(my_locale_ab_month_names_sq_AL) - 1, "",
    my_locale_ab_month_names_sq_AL, nullptr};
static TYPELIB my_locale_typelib_day_names_sq_AL = {
    array_elements(my_locale_day_names_sq_AL) - 1, "",
    my_locale_day_names_sq_AL, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_sq_AL = {
    array_elements(my_locale_ab_day_names_sq_AL) - 1, "",
    my_locale_ab_day_names_sq_AL, nullptr};
MY_LOCALE my_locale_sq_AL(47, "sq_AL", "Albanian - Albania", false,
                          &my_locale_typelib_month_names_sq_AL,
                          &my_locale_typelib_ab_month_names_sq_AL,
                          &my_locale_typelib_day_names_sq_AL,
                          &my_locale_typelib_ab_day_names_sq_AL, 7, 10,
                          ',',    /* decimal point sq_AL */
                          '.',    /* thousands_sep sq_AL */
                          "\x03", /* grouping      sq_AL */
                          &global_errmsgs[en_US]);
/***** LOCALE END sq_AL *****/

/***** LOCALE BEGIN sr_RS: Serbian - Serbia *****/
static const char *my_locale_month_names_sr_RS[13] = {
    "januar", "februar",   "mart",    "april",    "maj",      "juni", "juli",
    "avgust", "septembar", "oktobar", "novembar", "decembar", NullS};
static const char *my_locale_ab_month_names_sr_RS[13] = {
    "jan", "feb", "mar", "apr", "maj", "jun", "jul",
    "avg", "sep", "okt", "nov", "dec", NullS};
static const char *my_locale_day_names_sr_RS[8] = {
    "ponedeljak", "utorak", "sreda",   "četvrtak",
    "petak",      "subota", "nedelja", NullS};
static const char *my_locale_ab_day_names_sr_RS[8] = {
    "pon", "uto", "sre", "čet", "pet", "sub", "ned", NullS};
static TYPELIB my_locale_typelib_month_names_sr_RS = {
    array_elements(my_locale_month_names_sr_RS) - 1, "",
    my_locale_month_names_sr_RS, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_sr_RS = {
    array_elements(my_locale_ab_month_names_sr_RS) - 1, "",
    my_locale_ab_month_names_sr_RS, nullptr};
static TYPELIB my_locale_typelib_day_names_sr_RS = {
    array_elements(my_locale_day_names_sr_RS) - 1, "",
    my_locale_day_names_sr_RS, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_sr_RS = {
    array_elements(my_locale_ab_day_names_sr_RS) - 1, "",
    my_locale_ab_day_names_sr_RS, nullptr};
MY_LOCALE my_locale_sr_YU /* Deprecated, use sr_RS instead */
    (48, "sr_YU", "Serbian - Yugoslavia", false,
     &my_locale_typelib_month_names_sr_RS,
     &my_locale_typelib_ab_month_names_sr_RS,
     &my_locale_typelib_day_names_sr_RS, &my_locale_typelib_ab_day_names_sr_RS,
     9, 10, '.', /* decimal point sr_RS */
     '\0',       /* thousands_sep sr_RS */
     "\x80",     /* grouping      sr_RS */
     &global_errmsgs[sr_RS]);

MY_LOCALE my_locale_sr_RS(48, "sr_RS", "Serbian - Serbia", false,
                          &my_locale_typelib_month_names_sr_RS,
                          &my_locale_typelib_ab_month_names_sr_RS,
                          &my_locale_typelib_day_names_sr_RS,
                          &my_locale_typelib_ab_day_names_sr_RS, 9, 10,
                          '.',    /* decimal point sr_RS */
                          '\0',   /* thousands_sep sr_RS */
                          "\x80", /* grouping      sr_RS */
                          &global_errmsgs[sr_RS]);
/***** LOCALE END sr_RS *****/

/***** LOCALE BEGIN sv_SE: Swedish - Sweden *****/
static const char *my_locale_month_names_sv_SE[13] = {
    "januari", "februari",  "mars",    "april",    "maj",      "juni", "juli",
    "augusti", "september", "oktober", "november", "december", NullS};
static const char *my_locale_ab_month_names_sv_SE[13] = {
    "jan", "feb", "mar", "apr", "maj", "jun", "jul",
    "aug", "sep", "okt", "nov", "dec", NullS};
static const char *my_locale_day_names_sv_SE[8] = {
    "måndag", "tisdag", "onsdag", "torsdag",
    "fredag", "lördag", "söndag", NullS};
static const char *my_locale_ab_day_names_sv_SE[8] = {
    "mån", "tis", "ons", "tor", "fre", "lör", "sön", NullS};
static TYPELIB my_locale_typelib_month_names_sv_SE = {
    array_elements(my_locale_month_names_sv_SE) - 1, "",
    my_locale_month_names_sv_SE, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_sv_SE = {
    array_elements(my_locale_ab_month_names_sv_SE) - 1, "",
    my_locale_ab_month_names_sv_SE, nullptr};
static TYPELIB my_locale_typelib_day_names_sv_SE = {
    array_elements(my_locale_day_names_sv_SE) - 1, "",
    my_locale_day_names_sv_SE, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_sv_SE = {
    array_elements(my_locale_ab_day_names_sv_SE) - 1, "",
    my_locale_ab_day_names_sv_SE, nullptr};
MY_LOCALE my_locale_sv_SE(3, "sv_SE", "Swedish - Sweden", false,
                          &my_locale_typelib_month_names_sv_SE,
                          &my_locale_typelib_ab_month_names_sv_SE,
                          &my_locale_typelib_day_names_sv_SE,
                          &my_locale_typelib_ab_day_names_sv_SE, 9, 7,
                          ',',        /* decimal point sv_SE */
                          ' ',        /* thousands_sep sv_SE */
                          "\x03\x03", /* grouping      sv_SE */
                          &global_errmsgs[sv_SE]);
/***** LOCALE END sv_SE *****/

/***** LOCALE BEGIN ta_IN: Tamil - India *****/
static const char *my_locale_month_names_ta_IN[13] = {
    "ஜனவரி", "பெப்ரவரி", "மார்ச்",   "ஏப்ரல்",  "மே",      "ஜூன்", "ஜூலை",
    "ஆகஸ்ட்",  "செப்டம்பர்", "அக்டோபர்", "நவம்பர்", "டிசம்பர்r", NullS};
static const char *my_locale_ab_month_names_ta_IN[13] = {
    "ஜனவரி", "பெப்ரவரி", "மார்ச்",   "ஏப்ரல்",  "மே",      "ஜூன்", "ஜூலை",
    "ஆகஸ்ட்",  "செப்டம்பர்", "அக்டோபர்", "நவம்பர்", "டிசம்பர்r", NullS};
static const char *my_locale_day_names_ta_IN[8] = {
    "திங்கள்", "செவ்வாய்", "புதன்", "வியாழன்", "வெள்ளி", "சனி", "ஞாயிறு", NullS};
static const char *my_locale_ab_day_names_ta_IN[8] = {"த", "ச", "ப", "வ",
                                                      "வ", "ச", "ஞ", NullS};
static TYPELIB my_locale_typelib_month_names_ta_IN = {
    array_elements(my_locale_month_names_ta_IN) - 1, "",
    my_locale_month_names_ta_IN, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ta_IN = {
    array_elements(my_locale_ab_month_names_ta_IN) - 1, "",
    my_locale_ab_month_names_ta_IN, nullptr};
static TYPELIB my_locale_typelib_day_names_ta_IN = {
    array_elements(my_locale_day_names_ta_IN) - 1, "",
    my_locale_day_names_ta_IN, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ta_IN = {
    array_elements(my_locale_ab_day_names_ta_IN) - 1, "",
    my_locale_ab_day_names_ta_IN, nullptr};
MY_LOCALE my_locale_ta_IN(49, "ta_IN", "Tamil - India", false,
                          &my_locale_typelib_month_names_ta_IN,
                          &my_locale_typelib_ab_month_names_ta_IN,
                          &my_locale_typelib_day_names_ta_IN,
                          &my_locale_typelib_ab_day_names_ta_IN, 10, 8,
                          '.',        /* decimal point ta_IN */
                          ',',        /* thousands_sep ta_IN */
                          "\x03\x02", /* grouping      ta_IN */
                          &global_errmsgs[en_US]);
/***** LOCALE END ta_IN *****/

/***** LOCALE BEGIN te_IN: Telugu - India *****/
static const char *my_locale_month_names_te_IN[13] = {
    "జనవరి",  "ఫిబ్రవరి",  "మార్చి",   "ఏప్రిల్",  "మే",     "జూన్", "జూలై",
    "ఆగస్టు", "సెప్టెంబర్", "అక్టోబర్", "నవంబర్", "డిసెంబర్", NullS};
static const char *my_locale_ab_month_names_te_IN[13] = {
    "జనవరి",  "ఫిబ్రవరి",  "మార్చి",   "ఏప్రిల్",  "మే",     "జూన్", "జూలై",
    "ఆగస్టు", "సెప్టెంబర్", "అక్టోబర్", "నవంబర్", "డిసెంబర్", NullS};
static const char *my_locale_day_names_te_IN[8] = {
    "సోమవారం",   "మంగళవారం", "బుధవారం", "గురువారం",
    "శుక్రవారం", "శనివారం",   "ఆదివారం",  NullS};
static const char *my_locale_ab_day_names_te_IN[8] = {
    "సోమ", "మంగళ", "బుధ", "గురు", "శుక్ర", "శని", "ఆది", NullS};
static TYPELIB my_locale_typelib_month_names_te_IN = {
    array_elements(my_locale_month_names_te_IN) - 1, "",
    my_locale_month_names_te_IN, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_te_IN = {
    array_elements(my_locale_ab_month_names_te_IN) - 1, "",
    my_locale_ab_month_names_te_IN, nullptr};
static TYPELIB my_locale_typelib_day_names_te_IN = {
    array_elements(my_locale_day_names_te_IN) - 1, "",
    my_locale_day_names_te_IN, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_te_IN = {
    array_elements(my_locale_ab_day_names_te_IN) - 1, "",
    my_locale_ab_day_names_te_IN, nullptr};
MY_LOCALE my_locale_te_IN(50, "te_IN", "Telugu - India", false,
                          &my_locale_typelib_month_names_te_IN,
                          &my_locale_typelib_ab_month_names_te_IN,
                          &my_locale_typelib_day_names_te_IN,
                          &my_locale_typelib_ab_day_names_te_IN, 10, 9,
                          '.',        /* decimal point te_IN */
                          ',',        /* thousands_sep te_IN */
                          "\x03\x02", /* grouping      te_IN */
                          &global_errmsgs[en_US]);
/***** LOCALE END te_IN *****/

/***** LOCALE BEGIN th_TH: Thai - Thailand *****/
static const char *my_locale_month_names_th_TH[13] = {
    "มกราคม", "กุมภาพันธ์", "มีนาคม", "เมษายน",   "พฤษภาคม", "มิถุนายน", "กรกฎาคม",
    "สิงหาคม", "กันยายน",  "ตุลาคม", "พฤศจิกายน", "ธันวาคม",  NullS};
static const char *my_locale_ab_month_names_th_TH[13] = {
    "ม.ค.", "ก.พ.", "มี.ค.", "เม.ย.", "พ.ค.", "มิ.ย.", "ก.ค.",
    "ส.ค.", "ก.ย.", "ต.ค.", "พ.ย.",  "ธ.ค.", NullS};
static const char *my_locale_day_names_th_TH[8] = {
    "จันทร์", "อังคาร", "พุธ", "พฤหัสบดี", "ศุกร์", "เสาร์", "อาทิตย์", NullS};
static const char *my_locale_ab_day_names_th_TH[8] = {"จ.", "อ.", "พ.",  "พฤ.",
                                                      "ศ.", "ส.", "อา.", NullS};
static TYPELIB my_locale_typelib_month_names_th_TH = {
    array_elements(my_locale_month_names_th_TH) - 1, "",
    my_locale_month_names_th_TH, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_th_TH = {
    array_elements(my_locale_ab_month_names_th_TH) - 1, "",
    my_locale_ab_month_names_th_TH, nullptr};
static TYPELIB my_locale_typelib_day_names_th_TH = {
    array_elements(my_locale_day_names_th_TH) - 1, "",
    my_locale_day_names_th_TH, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_th_TH = {
    array_elements(my_locale_ab_day_names_th_TH) - 1, "",
    my_locale_ab_day_names_th_TH, nullptr};
MY_LOCALE my_locale_th_TH(51, "th_TH", "Thai - Thailand", false,
                          &my_locale_typelib_month_names_th_TH,
                          &my_locale_typelib_ab_month_names_th_TH,
                          &my_locale_typelib_day_names_th_TH,
                          &my_locale_typelib_ab_day_names_th_TH, 10, 8,
                          '.',    /* decimal point th_TH */
                          ',',    /* thousands_sep th_TH */
                          "\x03", /* grouping      th_TH */
                          &global_errmsgs[en_US]);
/***** LOCALE END th_TH *****/

/***** LOCALE BEGIN tr_TR: Turkish - Turkey *****/
static const char *my_locale_month_names_tr_TR[13] = {
    "Ocak",    "Şubat", "Mart", "Nisan", "Mayıs",  "Haziran", "Temmuz",
    "Ağustos", "Eylül", "Ekim", "Kasım", "Aralık", NullS};
static const char *my_locale_ab_month_names_tr_TR[13] = {
    "Oca", "Şub", "Mar", "Nis", "May", "Haz", "Tem",
    "Ağu", "Eyl", "Eki", "Kas", "Ara", NullS};
static const char *my_locale_day_names_tr_TR[8] = {
    "Pazartesi", "Salı",      "Çarşamba", "Perşembe",
    "Cuma",      "Cumartesi", "Pazar",    NullS};
static const char *my_locale_ab_day_names_tr_TR[8] = {
    "Pzt", "Sal", "Çrş", "Prş", "Cum", "Cts", "Paz", NullS};
static TYPELIB my_locale_typelib_month_names_tr_TR = {
    array_elements(my_locale_month_names_tr_TR) - 1, "",
    my_locale_month_names_tr_TR, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_tr_TR = {
    array_elements(my_locale_ab_month_names_tr_TR) - 1, "",
    my_locale_ab_month_names_tr_TR, nullptr};
static TYPELIB my_locale_typelib_day_names_tr_TR = {
    array_elements(my_locale_day_names_tr_TR) - 1, "",
    my_locale_day_names_tr_TR, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_tr_TR = {
    array_elements(my_locale_ab_day_names_tr_TR) - 1, "",
    my_locale_ab_day_names_tr_TR, nullptr};
MY_LOCALE my_locale_tr_TR(52, "tr_TR", "Turkish - Turkey", false,
                          &my_locale_typelib_month_names_tr_TR,
                          &my_locale_typelib_ab_month_names_tr_TR,
                          &my_locale_typelib_day_names_tr_TR,
                          &my_locale_typelib_ab_day_names_tr_TR, 7, 9,
                          ',',        /* decimal point tr_TR */
                          '.',        /* thousands_sep tr_TR */
                          "\x03\x03", /* grouping      tr_TR */
                          &global_errmsgs[en_US]);
/***** LOCALE END tr_TR *****/

/***** LOCALE BEGIN uk_UA: Ukrainian - Ukraine *****/
static const char *my_locale_month_names_uk_UA[13] = {
    "Січень",   "Лютий",   "Березень", "Квітень",  "Травень",
    "Червень",  "Липень",  "Серпень",  "Вересень", "Жовтень",
    "Листопад", "Грудень", NullS};
static const char *my_locale_ab_month_names_uk_UA[13] = {
    "Січ", "Лют", "Бер", "Кві", "Тра", "Чер", "Лип",
    "Сер", "Вер", "Жов", "Лис", "Гру", NullS};
static const char *my_locale_day_names_uk_UA[8] = {
    "Понеділок", "Вівторок", "Середа", "Четвер",
    "П'ятниця",  "Субота",   "Неділя", NullS};
static const char *my_locale_ab_day_names_uk_UA[8] = {
    "Пнд", "Втр", "Срд", "Чтв", "Птн", "Сбт", "Ндл", NullS};
static TYPELIB my_locale_typelib_month_names_uk_UA = {
    array_elements(my_locale_month_names_uk_UA) - 1, "",
    my_locale_month_names_uk_UA, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_uk_UA = {
    array_elements(my_locale_ab_month_names_uk_UA) - 1, "",
    my_locale_ab_month_names_uk_UA, nullptr};
static TYPELIB my_locale_typelib_day_names_uk_UA = {
    array_elements(my_locale_day_names_uk_UA) - 1, "",
    my_locale_day_names_uk_UA, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_uk_UA = {
    array_elements(my_locale_ab_day_names_uk_UA) - 1, "",
    my_locale_ab_day_names_uk_UA, nullptr};
MY_LOCALE my_locale_uk_UA(53, "uk_UA", "Ukrainian - Ukraine", false,
                          &my_locale_typelib_month_names_uk_UA,
                          &my_locale_typelib_ab_month_names_uk_UA,
                          &my_locale_typelib_day_names_uk_UA,
                          &my_locale_typelib_ab_day_names_uk_UA, 8, 9,
                          ',',        /* decimal point uk_UA */
                          '.',        /* thousands_sep uk_UA */
                          "\x03\x03", /* grouping      uk_UA */
                          &global_errmsgs[uk_UA]);
/***** LOCALE END uk_UA *****/

/***** LOCALE BEGIN ur_PK: Urdu - Pakistan *****/
static const char *my_locale_month_names_ur_PK[13] = {
    "جنوري", "فروري", "مارچ",   "اپريل", "مٓی",    "جون", "جولاي",
    "اگست",  "ستمبر", "اكتوبر", "نومبر", "دسمبر", NullS};
static const char *my_locale_ab_month_names_ur_PK[13] = {
    "جنوري", "فروري", "مارچ",   "اپريل", "مٓی",    "جون", "جولاي",
    "اگست",  "ستمبر", "اكتوبر", "نومبر", "دسمبر", NullS};
static const char *my_locale_day_names_ur_PK[8] = {
    "پير", "منگل", "بدھ", "جمعرات", "جمعه", "هفته", "اتوار", NullS};
static const char *my_locale_ab_day_names_ur_PK[8] = {
    "پير", "منگل", "بدھ", "جمعرات", "جمعه", "هفته", "اتوار", NullS};
static TYPELIB my_locale_typelib_month_names_ur_PK = {
    array_elements(my_locale_month_names_ur_PK) - 1, "",
    my_locale_month_names_ur_PK, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_ur_PK = {
    array_elements(my_locale_ab_month_names_ur_PK) - 1, "",
    my_locale_ab_month_names_ur_PK, nullptr};
static TYPELIB my_locale_typelib_day_names_ur_PK = {
    array_elements(my_locale_day_names_ur_PK) - 1, "",
    my_locale_day_names_ur_PK, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_ur_PK = {
    array_elements(my_locale_ab_day_names_ur_PK) - 1, "",
    my_locale_ab_day_names_ur_PK, nullptr};
MY_LOCALE my_locale_ur_PK(54, "ur_PK", "Urdu - Pakistan", false,
                          &my_locale_typelib_month_names_ur_PK,
                          &my_locale_typelib_ab_month_names_ur_PK,
                          &my_locale_typelib_day_names_ur_PK,
                          &my_locale_typelib_ab_day_names_ur_PK, 6, 6,
                          '.',        /* decimal point ur_PK */
                          ',',        /* thousands_sep ur_PK */
                          "\x03\x03", /* grouping      ur_PK */
                          &global_errmsgs[en_US]);
/***** LOCALE END ur_PK *****/

/***** LOCALE BEGIN vi_VN: Vietnamese - Vietnam *****/
static const char *my_locale_month_names_vi_VN[13] = {
    "Tháng một",      "Tháng hai",      "Tháng ba",  "Tháng tư",   "Tháng năm",
    "Tháng sáu",      "Tháng bảy",      "Tháng tám", "Tháng chín", "Tháng mười",
    "Tháng mười một", "Tháng mười hai", NullS};
static const char *my_locale_ab_month_names_vi_VN[13] = {
    "Thg 1", "Thg 2", "Thg 3",  "Thg 4",  "Thg 5",  "Thg 6", "Thg 7",
    "Thg 8", "Thg 9", "Thg 10", "Thg 11", "Thg 12", NullS};
static const char *my_locale_day_names_vi_VN[8] = {
    "Thứ hai ", "Thứ ba ",  "Thứ tư ",   "Thứ năm ",
    "Thứ sáu ", "Thứ bảy ", "Chủ nhật ", NullS};
static const char *my_locale_ab_day_names_vi_VN[8] = {
    "Th 2 ", "Th 3 ", "Th 4 ", "Th 5 ", "Th 6 ", "Th 7 ", "CN ", NullS};
static TYPELIB my_locale_typelib_month_names_vi_VN = {
    array_elements(my_locale_month_names_vi_VN) - 1, "",
    my_locale_month_names_vi_VN, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_vi_VN = {
    array_elements(my_locale_ab_month_names_vi_VN) - 1, "",
    my_locale_ab_month_names_vi_VN, nullptr};
static TYPELIB my_locale_typelib_day_names_vi_VN = {
    array_elements(my_locale_day_names_vi_VN) - 1, "",
    my_locale_day_names_vi_VN, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_vi_VN = {
    array_elements(my_locale_ab_day_names_vi_VN) - 1, "",
    my_locale_ab_day_names_vi_VN, nullptr};
MY_LOCALE my_locale_vi_VN(55, "vi_VN", "Vietnamese - Vietnam", false,
                          &my_locale_typelib_month_names_vi_VN,
                          &my_locale_typelib_ab_month_names_vi_VN,
                          &my_locale_typelib_day_names_vi_VN,
                          &my_locale_typelib_ab_day_names_vi_VN, 16, 11,
                          ',',        /* decimal point vi_VN */
                          '.',        /* thousands_sep vi_VN */
                          "\x03\x03", /* grouping      vi_VN */
                          &global_errmsgs[en_US]);
/***** LOCALE END vi_VN *****/

/***** LOCALE BEGIN zh_CN: Chinese - Peoples Republic of China *****/
static const char *my_locale_month_names_zh_CN[13] = {
    "一月", "二月", "三月", "四月",   "五月",   "六月", "七月",
    "八月", "九月", "十月", "十一月", "十二月", NullS};
static const char *my_locale_ab_month_names_zh_CN[13] = {
    " 1月", " 2月", " 3月", " 4月", " 5月", " 6月", " 7月",
    " 8月", " 9月", "10月", "11月", "12月", NullS};
static const char *my_locale_day_names_zh_CN[8] = {"星期一", "星期二", "星期三",
                                                   "星期四", "星期五", "星期六",
                                                   "星期日", NullS};
static const char *my_locale_ab_day_names_zh_CN[8] = {"一", "二", "三", "四",
                                                      "五", "六", "日", NullS};
static TYPELIB my_locale_typelib_month_names_zh_CN = {
    array_elements(my_locale_month_names_zh_CN) - 1, "",
    my_locale_month_names_zh_CN, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_zh_CN = {
    array_elements(my_locale_ab_month_names_zh_CN) - 1, "",
    my_locale_ab_month_names_zh_CN, nullptr};
static TYPELIB my_locale_typelib_day_names_zh_CN = {
    array_elements(my_locale_day_names_zh_CN) - 1, "",
    my_locale_day_names_zh_CN, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_zh_CN = {
    array_elements(my_locale_ab_day_names_zh_CN) - 1, "",
    my_locale_ab_day_names_zh_CN, nullptr};
MY_LOCALE my_locale_zh_CN(56, "zh_CN", "Chinese - Peoples Republic of China",
                          false, &my_locale_typelib_month_names_zh_CN,
                          &my_locale_typelib_ab_month_names_zh_CN,
                          &my_locale_typelib_day_names_zh_CN,
                          &my_locale_typelib_ab_day_names_zh_CN, 3, 3,
                          '.',    /* decimal point zh_CN */
                          ',',    /* thousands_sep zh_CN */
                          "\x03", /* grouping      zh_CN */
                          &global_errmsgs[en_US]);
/***** LOCALE END zh_CN *****/

/***** LOCALE BEGIN zh_TW: Chinese - Taiwan *****/
static const char *my_locale_month_names_zh_TW[13] = {
    "一月", "二月", "三月", "四月",   "五月",   "六月", "七月",
    "八月", "九月", "十月", "十一月", "十二月", NullS};
static const char *my_locale_ab_month_names_zh_TW[13] = {
    " 1月", " 2月", " 3月", " 4月", " 5月", " 6月", " 7月",
    " 8月", " 9月", "10月", "11月", "12月", NullS};
static const char *my_locale_day_names_zh_TW[8] = {
    "週一", "週二", "週三", "週四", "週五", "週六", "週日", NullS};
static const char *my_locale_ab_day_names_zh_TW[8] = {"一", "二", "三", "四",
                                                      "五", "六", "日", NullS};
static TYPELIB my_locale_typelib_month_names_zh_TW = {
    array_elements(my_locale_month_names_zh_TW) - 1, "",
    my_locale_month_names_zh_TW, nullptr};
static TYPELIB my_locale_typelib_ab_month_names_zh_TW = {
    array_elements(my_locale_ab_month_names_zh_TW) - 1, "",
    my_locale_ab_month_names_zh_TW, nullptr};
static TYPELIB my_locale_typelib_day_names_zh_TW = {
    array_elements(my_locale_day_names_zh_TW) - 1, "",
    my_locale_day_names_zh_TW, nullptr};
static TYPELIB my_locale_typelib_ab_day_names_zh_TW = {
    array_elements(my_locale_ab_day_names_zh_TW) - 1, "",
    my_locale_ab_day_names_zh_TW, nullptr};
MY_LOCALE my_locale_zh_TW(57, "zh_TW", "Chinese - Taiwan", false,
                          &my_locale_typelib_month_names_zh_TW,
                          &my_locale_typelib_ab_month_names_zh_TW,
                          &my_locale_typelib_day_names_zh_TW,
                          &my_locale_typelib_ab_day_names_zh_TW, 3, 2,
                          '.',    /* decimal point zh_TW */
                          ',',    /* thousands_sep zh_TW */
                          "\x03", /* grouping      zh_TW */
                          &global_errmsgs[en_US]);
/***** LOCALE END zh_TW *****/

/***** LOCALE BEGIN ar_DZ: Arabic - Algeria *****/
MY_LOCALE my_locale_ar_DZ(58, "ar_DZ", "Arabic - Algeria", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_DZ */
                          ',',    /* thousands_sep ar_DZ */
                          "\x03", /* grouping      ar_DZ */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_DZ *****/

/***** LOCALE BEGIN ar_EG: Arabic - Egypt *****/
MY_LOCALE my_locale_ar_EG(59, "ar_EG", "Arabic - Egypt", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_EG */
                          ',',    /* thousands_sep ar_EG */
                          "\x03", /* grouping      ar_EG */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_EG *****/

/***** LOCALE BEGIN ar_IN: Arabic - Iran *****/
MY_LOCALE my_locale_ar_IN(60, "ar_IN", "Arabic - Iran", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_IN */
                          ',',    /* thousands_sep ar_IN */
                          "\x03", /* grouping      ar_IN */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_IN *****/

/***** LOCALE BEGIN ar_IQ: Arabic - Iraq *****/
MY_LOCALE my_locale_ar_IQ(61, "ar_IQ", "Arabic - Iraq", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_IQ */
                          ',',    /* thousands_sep ar_IQ */
                          "\x03", /* grouping      ar_IQ */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_IQ *****/

/***** LOCALE BEGIN ar_KW: Arabic - Kuwait *****/
MY_LOCALE my_locale_ar_KW(62, "ar_KW", "Arabic - Kuwait", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_KW */
                          ',',    /* thousands_sep ar_KW */
                          "\x03", /* grouping      ar_KW */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_KW *****/

/***** LOCALE BEGIN ar_LB: Arabic - Lebanon *****/
MY_LOCALE my_locale_ar_LB(63, "ar_LB", "Arabic - Lebanon", false,
                          &my_locale_typelib_month_names_ar_JO,
                          &my_locale_typelib_ab_month_names_ar_JO,
                          &my_locale_typelib_day_names_ar_JO,
                          &my_locale_typelib_ab_day_names_ar_JO, 12, 8,
                          '.',    /* decimal point ar_LB */
                          ',',    /* thousands_sep ar_LB */
                          "\x03", /* grouping      ar_LB */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_LB *****/

/***** LOCALE BEGIN ar_LY: Arabic - Libya *****/
MY_LOCALE my_locale_ar_LY(64, "ar_LY", "Arabic - Libya", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_LY */
                          ',',    /* thousands_sep ar_LY */
                          "\x03", /* grouping      ar_LY */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_LY *****/

/***** LOCALE BEGIN ar_MA: Arabic - Morocco *****/
MY_LOCALE my_locale_ar_MA(65, "ar_MA", "Arabic - Morocco", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_MA */
                          ',',    /* thousands_sep ar_MA */
                          "\x03", /* grouping      ar_MA */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_MA *****/

/***** LOCALE BEGIN ar_OM: Arabic - Oman *****/
MY_LOCALE my_locale_ar_OM(66, "ar_OM", "Arabic - Oman", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_OM */
                          ',',    /* thousands_sep ar_OM */
                          "\x03", /* grouping      ar_OM */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_OM *****/

/***** LOCALE BEGIN ar_QA: Arabic - Qatar *****/
MY_LOCALE my_locale_ar_QA(67, "ar_QA", "Arabic - Qatar", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_QA */
                          ',',    /* thousands_sep ar_QA */
                          "\x03", /* grouping      ar_QA */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_QA *****/

/***** LOCALE BEGIN ar_SD: Arabic - Sudan *****/
MY_LOCALE my_locale_ar_SD(68, "ar_SD", "Arabic - Sudan", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_SD */
                          ',',    /* thousands_sep ar_SD */
                          "\x03", /* grouping      ar_SD */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_SD *****/

/***** LOCALE BEGIN ar_TN: Arabic - Tunisia *****/
MY_LOCALE my_locale_ar_TN(69, "ar_TN", "Arabic - Tunisia", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_TN */
                          ',',    /* thousands_sep ar_TN */
                          "\x03", /* grouping      ar_TN */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_TN *****/

/***** LOCALE BEGIN ar_YE: Arabic - Yemen *****/
MY_LOCALE my_locale_ar_YE(70, "ar_YE", "Arabic - Yemen", false,
                          &my_locale_typelib_month_names_ar_BH,
                          &my_locale_typelib_ab_month_names_ar_BH,
                          &my_locale_typelib_day_names_ar_BH,
                          &my_locale_typelib_ab_day_names_ar_BH, 6, 8,
                          '.',    /* decimal point ar_YE */
                          ',',    /* thousands_sep ar_YE */
                          "\x03", /* grouping      ar_YE */
                          &global_errmsgs[en_US]);
/***** LOCALE END ar_YE *****/

/***** LOCALE BEGIN de_BE: German - Belgium *****/
MY_LOCALE my_locale_de_BE(71, "de_BE", "German - Belgium", false,
                          &my_locale_typelib_month_names_de_DE,
                          &my_locale_typelib_ab_month_names_de_DE,
                          &my_locale_typelib_day_names_de_DE,
                          &my_locale_typelib_ab_day_names_de_DE, 9, 10,
                          ',',        /* decimal point de_BE */
                          '.',        /* thousands_sep de_BE */
                          "\x03\x03", /* grouping    de_BE */
                          &global_errmsgs[de_DE]);
/***** LOCALE END de_BE *****/

/***** LOCALE BEGIN de_CH: German - Switzerland *****/
MY_LOCALE my_locale_de_CH(72, "de_CH", "German - Switzerland", false,
                          &my_locale_typelib_month_names_de_DE,
                          &my_locale_typelib_ab_month_names_de_DE,
                          &my_locale_typelib_day_names_de_DE,
                          &my_locale_typelib_ab_day_names_de_DE, 9, 10,
                          '.',        /* decimal point de_CH */
                          '\'',       /* thousands_sep de_CH */
                          "\x03\x03", /* grouping      de_CH */
                          &global_errmsgs[de_DE]);
/***** LOCALE END de_CH *****/

/***** LOCALE BEGIN de_LU: German - Luxembourg *****/
MY_LOCALE my_locale_de_LU(73, "de_LU", "German - Luxembourg", false,
                          &my_locale_typelib_month_names_de_DE,
                          &my_locale_typelib_ab_month_names_de_DE,
                          &my_locale_typelib_day_names_de_DE,
                          &my_locale_typelib_ab_day_names_de_DE, 9, 10,
                          ',',        /* decimal point de_LU */
                          '.',        /* thousands_sep de_LU */
                          "\x03\x03", /* grouping      de_LU */
                          &global_errmsgs[de_DE]);
/***** LOCALE END de_LU *****/

/***** LOCALE BEGIN en_AU: English - Australia *****/
MY_LOCALE my_locale_en_AU(74, "en_AU", "English - Australia", true,
                          &my_locale_typelib_month_names_en_US,
                          &my_locale_typelib_ab_month_names_en_US,
                          &my_locale_typelib_day_names_en_US,
                          &my_locale_typelib_ab_day_names_en_US, 9, 9,
                          '.',        /* decimal point en_AU */
                          ',',        /* thousands_sep en_AU */
                          "\x03\x03", /* grouping      en_AU */
                          &global_errmsgs[en_US]);
/***** LOCALE END en_AU *****/

/***** LOCALE BEGIN en_CA: English - Canada *****/
MY_LOCALE my_locale_en_CA(75, "en_CA", "English - Canada", true,
                          &my_locale_typelib_month_names_en_US,
                          &my_locale_typelib_ab_month_names_en_US,
                          &my_locale_typelib_day_names_en_US,
                          &my_locale_typelib_ab_day_names_en_US, 9, 9,
                          '.',        /* decimal point en_CA */
                          ',',        /* thousands_sep en_CA */
                          "\x03\x03", /* grouping      en_CA */
                          &global_errmsgs[en_US]);
/***** LOCALE END en_CA *****/

/***** LOCALE BEGIN en_GB: English - United Kingdom *****/
MY_LOCALE my_locale_en_GB(1, "en_GB", "English - United Kingdom", true,
                          &my_locale_typelib_month_names_en_US,
                          &my_locale_typelib_ab_month_names_en_US,
                          &my_locale_typelib_day_names_en_US,
                          &my_locale_typelib_ab_day_names_en_US, 9, 9,
                          '.',        /* decimal point en_GB */
                          ',',        /* thousands_sep en_GB */
                          "\x03\x03", /* grouping      en_GB */
                          &global_errmsgs[en_US]);
/***** LOCALE END en_GB *****/

/***** LOCALE BEGIN en_IN: English - India *****/
MY_LOCALE my_locale_en_IN(76, "en_IN", "English - India", true,
                          &my_locale_typelib_month_names_en_US,
                          &my_locale_typelib_ab_month_names_en_US,
                          &my_locale_typelib_day_names_en_US,
                          &my_locale_typelib_ab_day_names_en_US, 9, 9,
                          '.',        /* decimal point en_IN */
                          ',',        /* thousands_sep en_IN */
                          "\x03\x02", /* grouping      en_IN */
                          &global_errmsgs[en_US]);
/***** LOCALE END en_IN *****/

/***** LOCALE BEGIN en_NZ: English - New Zealand *****/
MY_LOCALE my_locale_en_NZ(77, "en_NZ", "English - New Zealand", true,
                          &my_locale_typelib_month_names_en_US,
                          &my_locale_typelib_ab_month_names_en_US,
                          &my_locale_typelib_day_names_en_US,
                          &my_locale_typelib_ab_day_names_en_US, 9, 9,
                          '.',        /* decimal point en_NZ */
                          ',',        /* thousands_sep en_NZ */
                          "\x03\x03", /* grouping      en_NZ */
                          &global_errmsgs[en_US]);
/***** LOCALE END en_NZ *****/

/***** LOCALE BEGIN en_PH: English - Philippines *****/
MY_LOCALE my_locale_en_PH(78, "en_PH", "English - Philippines", true,
                          &my_locale_typelib_month_names_en_US,
                          &my_locale_typelib_ab_month_names_en_US,
                          &my_locale_typelib_day_names_en_US,
                          &my_locale_typelib_ab_day_names_en_US, 9, 9,
                          '.',    /* decimal point en_PH */
                          ',',    /* thousands_sep en_PH */
                          "\x03", /* grouping      en_PH */
                          &global_errmsgs[en_US]);
/***** LOCALE END en_PH *****/

/***** LOCALE BEGIN en_ZA: English - South Africa *****/
MY_LOCALE my_locale_en_ZA(79, "en_ZA", "English - South Africa", true,
                          &my_locale_typelib_month_names_en_US,
                          &my_locale_typelib_ab_month_names_en_US,
                          &my_locale_typelib_day_names_en_US,
                          &my_locale_typelib_ab_day_names_en_US, 9, 9,
                          '.',        /* decimal point en_ZA */
                          ',',        /* thousands_sep en_ZA */
                          "\x03\x03", /* grouping      en_ZA */
                          &global_errmsgs[en_US]);
/***** LOCALE END en_ZA *****/

/***** LOCALE BEGIN en_ZW: English - Zimbabwe *****/
MY_LOCALE my_locale_en_ZW(80, "en_ZW", "English - Zimbabwe", true,
                          &my_locale_typelib_month_names_en_US,
                          &my_locale_typelib_ab_month_names_en_US,
                          &my_locale_typelib_day_names_en_US,
                          &my_locale_typelib_ab_day_names_en_US, 9, 9,
                          '.',        /* decimal point en_ZW */
                          ',',        /* thousands_sep en_ZW */
                          "\x03\x03", /* grouping      en_ZW */
                          &global_errmsgs[en_US]);
/***** LOCALE END en_ZW *****/

/***** LOCALE BEGIN es_AR: Spanish - Argentina *****/
MY_LOCALE my_locale_es_AR(81, "es_AR", "Spanish - Argentina", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          ',',        /* decimal point es_AR */
                          '.',        /* thousands_sep es_AR */
                          "\x03\x03", /* grouping      es_AR */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_AR *****/

/***** LOCALE BEGIN es_BO: Spanish - Bolivia *****/
MY_LOCALE my_locale_es_BO(82, "es_BO", "Spanish - Bolivia", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          ',',        /* decimal point es_BO */
                          '\0',       /* thousands_sep es_BO */
                          "\x80\x80", /* grouping      es_BO */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_BO *****/

/***** LOCALE BEGIN es_CL: Spanish - Chile *****/
MY_LOCALE my_locale_es_CL(83, "es_CL", "Spanish - Chile", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          ',',        /* decimal point es_CL */
                          '\0',       /* thousands_sep es_CL */
                          "\x80\x80", /* grouping      es_CL */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_CL *****/

/***** LOCALE BEGIN es_CO: Spanish - Columbia *****/
MY_LOCALE my_locale_es_CO(84, "es_CO", "Spanish - Columbia", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          ',',        /* decimal point es_CO */
                          '\0',       /* thousands_sep es_CO */
                          "\x80\x80", /* grouping      es_CO */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_CO *****/

/***** LOCALE BEGIN es_CR: Spanish - Costa Rica *****/
MY_LOCALE my_locale_es_CR(85, "es_CR", "Spanish - Costa Rica", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          '.',        /* decimal point es_CR */
                          '\0',       /* thousands_sep es_CR */
                          "\x80\x80", /* grouping      es_CR */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_CR *****/

/***** LOCALE BEGIN es_DO: Spanish - Dominican Republic *****/
MY_LOCALE my_locale_es_DO(86, "es_DO", "Spanish - Dominican Republic", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          '.',        /* decimal point es_DO */
                          '\0',       /* thousands_sep es_DO */
                          "\x80\x80", /* grouping      es_DO */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_DO *****/

/***** LOCALE BEGIN es_EC: Spanish - Ecuador *****/
MY_LOCALE my_locale_es_EC(87, "es_EC", "Spanish - Ecuador", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          ',',        /* decimal point es_EC */
                          '\0',       /* thousands_sep es_EC */
                          "\x80\x80", /* grouping      es_EC */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_EC *****/

/***** LOCALE BEGIN es_GT: Spanish - Guatemala *****/
MY_LOCALE my_locale_es_GT(88, "es_GT", "Spanish - Guatemala", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          '.',        /* decimal point es_GT */
                          '\0',       /* thousands_sep es_GT */
                          "\x80\x80", /* grouping      es_GT */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_GT *****/

/***** LOCALE BEGIN es_HN: Spanish - Honduras *****/
MY_LOCALE my_locale_es_HN(89, "es_HN", "Spanish - Honduras", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          '.',        /* decimal point es_HN */
                          '\0',       /* thousands_sep es_HN */
                          "\x80\x80", /* grouping      es_HN */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_HN *****/

/***** LOCALE BEGIN es_MX: Spanish - Mexico *****/
MY_LOCALE my_locale_es_MX(90, "es_MX", "Spanish - Mexico", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          '.',        /* decimal point es_MX */
                          '\0',       /* thousands_sep es_MX */
                          "\x80\x80", /* grouping      es_MX */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_MX *****/

/***** LOCALE BEGIN es_NI: Spanish - Nicaragua *****/
MY_LOCALE my_locale_es_NI(91, "es_NI", "Spanish - Nicaragua", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          '.',        /* decimal point es_NI */
                          '\0',       /* thousands_sep es_NI */
                          "\x80\x80", /* grouping      es_NI */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_NI *****/

/***** LOCALE BEGIN es_PA: Spanish - Panama *****/
MY_LOCALE my_locale_es_PA(92, "es_PA", "Spanish - Panama", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          '.',        /* decimal point es_PA */
                          '\0',       /* thousands_sep es_PA */
                          "\x80\x80", /* grouping      es_PA */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_PA *****/

/***** LOCALE BEGIN es_PE: Spanish - Peru *****/
MY_LOCALE my_locale_es_PE(93, "es_PE", "Spanish - Peru", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          '.',        /* decimal point es_PE */
                          '\0',       /* thousands_sep es_PE */
                          "\x80\x80", /* grouping      es_PE */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_PE *****/

/***** LOCALE BEGIN es_PR: Spanish - Puerto Rico *****/
MY_LOCALE my_locale_es_PR(94, "es_PR", "Spanish - Puerto Rico", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          '.',        /* decimal point es_PR */
                          '\0',       /* thousands_sep es_PR */
                          "\x80\x80", /* grouping      es_PR */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_PR *****/

/***** LOCALE BEGIN es_PY: Spanish - Paraguay *****/
MY_LOCALE my_locale_es_PY(95, "es_PY", "Spanish - Paraguay", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          ',',        /* decimal point es_PY */
                          '\0',       /* thousands_sep es_PY */
                          "\x80\x80", /* grouping      es_PY */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_PY *****/

/***** LOCALE BEGIN es_SV: Spanish - El Salvador *****/
MY_LOCALE my_locale_es_SV(96, "es_SV", "Spanish - El Salvador", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          '.',        /* decimal point es_SV */
                          '\0',       /* thousands_sep es_SV */
                          "\x80\x80", /* grouping      es_SV */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_SV *****/

/***** LOCALE BEGIN es_US: Spanish - United States *****/
MY_LOCALE my_locale_es_US(97, "es_US", "Spanish - United States", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          '.',        /* decimal point es_US */
                          ',',        /* thousands_sep es_US */
                          "\x03\x03", /* grouping      es_US */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_US *****/

/***** LOCALE BEGIN es_UY: Spanish - Uruguay *****/
MY_LOCALE my_locale_es_UY(98, "es_UY", "Spanish - Uruguay", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          ',',        /* decimal point es_UY */
                          '\0',       /* thousands_sep es_UY */
                          "\x80\x80", /* grouping      es_UY */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_UY *****/

/***** LOCALE BEGIN es_VE: Spanish - Venezuela *****/
MY_LOCALE my_locale_es_VE(99, "es_VE", "Spanish - Venezuela", false,
                          &my_locale_typelib_month_names_es_ES,
                          &my_locale_typelib_ab_month_names_es_ES,
                          &my_locale_typelib_day_names_es_ES,
                          &my_locale_typelib_ab_day_names_es_ES, 10, 9,
                          ',',        /* decimal point es_VE */
                          '\0',       /* thousands_sep es_VE */
                          "\x80\x80", /* grouping      es_VE */
                          &global_errmsgs[es_ES]);
/***** LOCALE END es_VE *****/

/***** LOCALE BEGIN fr_BE: French - Belgium *****/
MY_LOCALE my_locale_fr_BE(100, "fr_BE", "French - Belgium", false,
                          &my_locale_typelib_month_names_fr_FR,
                          &my_locale_typelib_ab_month_names_fr_FR,
                          &my_locale_typelib_day_names_fr_FR,
                          &my_locale_typelib_ab_day_names_fr_FR, 9, 8,
                          ',',        /* decimal point fr_BE */
                          '.',        /* thousands_sep fr_BE */
                          "\x80\x80", /* grouping      fr_BE */
                          &global_errmsgs[fr_FR]);
/***** LOCALE END fr_BE *****/

/***** LOCALE BEGIN fr_CA: French - Canada *****/
MY_LOCALE my_locale_fr_CA(101, "fr_CA", "French - Canada", false,
                          &my_locale_typelib_month_names_fr_FR,
                          &my_locale_typelib_ab_month_names_fr_FR,
                          &my_locale_typelib_day_names_fr_FR,
                          &my_locale_typelib_ab_day_names_fr_FR, 9, 8,
                          ',',        /* decimal point fr_CA */
                          ' ',        /* thousands_sep fr_CA */
                          "\x80\x80", /* grouping      fr_CA */
                          &global_errmsgs[fr_FR]);
/***** LOCALE END fr_CA *****/

/***** LOCALE BEGIN fr_CH: French - Switzerland *****/
MY_LOCALE my_locale_fr_CH(102, "fr_CH", "French - Switzerland", false,
                          &my_locale_typelib_month_names_fr_FR,
                          &my_locale_typelib_ab_month_names_fr_FR,
                          &my_locale_typelib_day_names_fr_FR,
                          &my_locale_typelib_ab_day_names_fr_FR, 9, 8,
                          ',',        /* decimal point fr_CH */
                          '\0',       /* thousands_sep fr_CH */
                          "\x80\x80", /* grouping      fr_CH */
                          &global_errmsgs[fr_FR]);
/***** LOCALE END fr_CH *****/

/***** LOCALE BEGIN fr_LU: French - Luxembourg *****/
MY_LOCALE my_locale_fr_LU(103, "fr_LU", "French - Luxembourg", false,
                          &my_locale_typelib_month_names_fr_FR,
                          &my_locale_typelib_ab_month_names_fr_FR,
                          &my_locale_typelib_day_names_fr_FR,
                          &my_locale_typelib_ab_day_names_fr_FR, 9, 8,
                          ',',        /* decimal point fr_LU */
                          '\0',       /* thousands_sep fr_LU */
                          "\x80\x80", /* grouping      fr_LU */
                          &global_errmsgs[fr_FR]);
/***** LOCALE END fr_LU *****/

/***** LOCALE BEGIN it_IT: Italian - Italy *****/
MY_LOCALE my_locale_it_IT(104, "it_IT", "Italian - Italy", false,
                          &my_locale_typelib_month_names_it_CH,
                          &my_locale_typelib_ab_month_names_it_CH,
                          &my_locale_typelib_day_names_it_CH,
                          &my_locale_typelib_ab_day_names_it_CH, 9, 9,
                          ',',        /* decimal point it_IT */
                          '\0',       /* thousands_sep it_IT */
                          "\x80\x80", /* grouping      it_IT */
                          &global_errmsgs[it_IT]);
/***** LOCALE END it_IT *****/

/***** LOCALE BEGIN nl_BE: Dutch - Belgium *****/
MY_LOCALE my_locale_nl_BE(105, "nl_BE", "Dutch - Belgium", true,
                          &my_locale_typelib_month_names_nl_NL,
                          &my_locale_typelib_ab_month_names_nl_NL,
                          &my_locale_typelib_day_names_nl_NL,
                          &my_locale_typelib_ab_day_names_nl_NL, 9, 9,
                          ',',        /* decimal point nl_BE */
                          '.',        /* thousands_sep nl_BE */
                          "\x80\x80", /* grouping      nl_BE */
                          &global_errmsgs[nl_NL]);
/***** LOCALE END nl_BE *****/

/***** LOCALE BEGIN no_NO: Norwegian - Norway *****/
MY_LOCALE my_locale_no_NO(106, "no_NO", "Norwegian - Norway", false,
                          &my_locale_typelib_month_names_nb_NO,
                          &my_locale_typelib_ab_month_names_nb_NO,
                          &my_locale_typelib_day_names_nb_NO,
                          &my_locale_typelib_ab_day_names_nb_NO, 9, 7,
                          ',',        /* decimal point no_NO */
                          '.',        /* thousands_sep no_NO */
                          "\x03\x03", /* grouping      no_NO */
                          &global_errmsgs[no_NO]);
/***** LOCALE END no_NO *****/

/***** LOCALE BEGIN sv_FI: Swedish - Finland *****/
MY_LOCALE my_locale_sv_FI(107, "sv_FI", "Swedish - Finland", false,
                          &my_locale_typelib_month_names_sv_SE,
                          &my_locale_typelib_ab_month_names_sv_SE,
                          &my_locale_typelib_day_names_sv_SE,
                          &my_locale_typelib_ab_day_names_sv_SE, 9, 7,
                          ',',        /* decimal point sv_FI */
                          ' ',        /* thousands_sep sv_FI */
                          "\x03\x03", /* grouping      sv_FI */
                          &global_errmsgs[sv_SE]);
/***** LOCALE END sv_FI *****/

/***** LOCALE BEGIN zh_HK: Chinese - Hong Kong SAR *****/
MY_LOCALE my_locale_zh_HK(108, "zh_HK", "Chinese - Hong Kong SAR", false,
                          &my_locale_typelib_month_names_zh_CN,
                          &my_locale_typelib_ab_month_names_zh_CN,
                          &my_locale_typelib_day_names_zh_CN,
                          &my_locale_typelib_ab_day_names_zh_CN, 3, 3,
                          '.',    /* decimal point zh_HK */
                          ',',    /* thousands_sep zh_HK */
                          "\x03", /* grouping      zh_HK */
                          &global_errmsgs[en_US]);
/***** LOCALE END zh_HK *****/

/***** LOCALE BEGIN el_GR: Greek - Greece *****/
static const char *my_locale_month_names_el_GR[13] = {
    "Ιανουάριος", "Φεβρουάριος", "Μάρτιος",   "Απρίλιος",    "Μάιος",
    "Ιούνιος",    "Ιούλιος",     "Αύγουστος", "Σεπτέμβριος", "Οκτώβριος",
    "Νοέμβριος",  "Δεκέμβριος",  NullS};

static const char *my_locale_ab_month_names_el_GR[13] = {
    "Ιαν", "Φεβ", "Μάρ", "Απρ", "Μάι", "Ιούν", "Ιούλ",
    "Αύγ", "Σεπ", "Οκτ", "Νοέ", "Δεκ", NullS};

static const char *my_locale_day_names_el_GR[8] = {
    "Δευτέρα",   "Τρίτη",   "Τετάρτη", "Πέμπτη",
    "Παρασκευή", "Σάββατο", "Κυριακή", NullS};

static const char *my_locale_ab_day_names_el_GR[8] = {
    "Δευ", "Τρί", "Τετ", "Πέμ", "Παρ", "Σάβ", "Κυρ", NullS};

static TYPELIB my_locale_typelib_month_names_el_GR = {
    array_elements(my_locale_month_names_el_GR) - 1, "",
    my_locale_month_names_el_GR, nullptr};

static TYPELIB my_locale_typelib_ab_month_names_el_GR = {
    array_elements(my_locale_ab_month_names_el_GR) - 1, "",
    my_locale_ab_month_names_el_GR, nullptr};

static TYPELIB my_locale_typelib_day_names_el_GR = {
    array_elements(my_locale_day_names_el_GR) - 1, "",
    my_locale_day_names_el_GR, nullptr};

static TYPELIB my_locale_typelib_ab_day_names_el_GR = {
    array_elements(my_locale_ab_day_names_el_GR) - 1, "",
    my_locale_ab_day_names_el_GR, nullptr};

MY_LOCALE my_locale_el_GR(109, "el_GR", "Greek - Greece", false,
                          &my_locale_typelib_month_names_el_GR,
                          &my_locale_typelib_ab_month_names_el_GR,
                          &my_locale_typelib_day_names_el_GR,
                          &my_locale_typelib_ab_day_names_el_GR,
                          11,     /* max mon name length */
                          9,      /* max day name length */
                          ',',    /* decimal point el_GR */
                          '.',    /* thousands_sep el_GR */
                          "\x80", /* grouping      el_GR */
                          &global_errmsgs[el_GR]);
/***** LOCALE END el_GR *****/

/***** LOCALE BEGIN rm_CH: Romansh - Switzerland *****/
static const char *my_locale_month_names_rm_CH[13] = {
    "schaner",   "favrer",   "mars",  "avrigl",    "matg",
    "zercladur", "fanadur",  "avust", "settember", "october",
    "november",  "december", NullS};

static const char *my_locale_ab_month_names_rm_CH[13] = {
    "schan", "favr", "mars", "avr", "matg", "zercl", "fan",
    "avust", "sett", "oct",  "nov", "dec",  NullS};

static const char *my_locale_day_names_rm_CH[8] = {
    "glindesdi", "mardi", "mesemna",  "gievgia",
    "venderdi",  "sonda", "dumengia", NullS};

static const char *my_locale_ab_day_names_rm_CH[8] = {"gli", "ma", "me", "gie",
                                                      "ve",  "so", "du", NullS};

static TYPELIB my_locale_typelib_month_names_rm_CH = {
    array_elements(my_locale_month_names_rm_CH) - 1, "",
    my_locale_month_names_rm_CH, nullptr};

static TYPELIB my_locale_typelib_ab_month_names_rm_CH = {
    array_elements(my_locale_ab_month_names_rm_CH) - 1, "",
    my_locale_ab_month_names_rm_CH, nullptr};

static TYPELIB my_locale_typelib_day_names_rm_CH = {
    array_elements(my_locale_day_names_rm_CH) - 1, "",
    my_locale_day_names_rm_CH, nullptr};

static TYPELIB my_locale_typelib_ab_day_names_rm_CH = {
    array_elements(my_locale_ab_day_names_rm_CH) - 1, "",
    my_locale_ab_day_names_rm_CH, nullptr};

MY_LOCALE my_locale_rm_CH(110, "rm_CH", "Romansh - Switzerland", false,
                          &my_locale_typelib_month_names_rm_CH,
                          &my_locale_typelib_ab_month_names_rm_CH,
                          &my_locale_typelib_day_names_rm_CH,
                          &my_locale_typelib_ab_day_names_rm_CH,
                          9,          /* max mon name length */
                          9,          /* max day name length */
                          ',',        /* decimal point rm_CH */
                          '\'',       /* thousands_sep rm_CH */
                          "\x03\x03", /* grouping      rm_CH */
                          &global_errmsgs[en_US]);
/***** LOCALE END rm_CH *****/

/*
  The list of all locales.
  Note, locales must be ordered according to their
  numbers to make my_locale_by_number() work fast.
  Some debug asserts below check this.
*/
MY_LOCALE *my_locales[] = {
    &my_locale_en_US, &my_locale_en_GB, &my_locale_ja_JP, &my_locale_sv_SE,
    &my_locale_de_DE, &my_locale_fr_FR, &my_locale_ar_AE, &my_locale_ar_BH,
    &my_locale_ar_JO, &my_locale_ar_SA, &my_locale_ar_SY, &my_locale_be_BY,
    &my_locale_bg_BG, &my_locale_ca_ES, &my_locale_cs_CZ, &my_locale_da_DK,
    &my_locale_de_AT, &my_locale_es_ES, &my_locale_et_EE, &my_locale_eu_ES,
    &my_locale_fi_FI, &my_locale_fo_FO, &my_locale_gl_ES, &my_locale_gu_IN,
    &my_locale_he_IL, &my_locale_hi_IN, &my_locale_hr_HR, &my_locale_hu_HU,
    &my_locale_id_ID, &my_locale_is_IS, &my_locale_it_CH, &my_locale_ko_KR,
    &my_locale_lt_LT, &my_locale_lv_LV, &my_locale_mk_MK, &my_locale_mn_MN,
    &my_locale_ms_MY, &my_locale_nb_NO, &my_locale_nl_NL, &my_locale_pl_PL,
    &my_locale_pt_BR, &my_locale_pt_PT, &my_locale_ro_RO, &my_locale_ru_RU,
    &my_locale_ru_UA, &my_locale_sk_SK, &my_locale_sl_SI, &my_locale_sq_AL,
    &my_locale_sr_RS, &my_locale_ta_IN, &my_locale_te_IN, &my_locale_th_TH,
    &my_locale_tr_TR, &my_locale_uk_UA, &my_locale_ur_PK, &my_locale_vi_VN,
    &my_locale_zh_CN, &my_locale_zh_TW, &my_locale_ar_DZ, &my_locale_ar_EG,
    &my_locale_ar_IN, &my_locale_ar_IQ, &my_locale_ar_KW, &my_locale_ar_LB,
    &my_locale_ar_LY, &my_locale_ar_MA, &my_locale_ar_OM, &my_locale_ar_QA,
    &my_locale_ar_SD, &my_locale_ar_TN, &my_locale_ar_YE, &my_locale_de_BE,
    &my_locale_de_CH, &my_locale_de_LU, &my_locale_en_AU, &my_locale_en_CA,
    &my_locale_en_IN, &my_locale_en_NZ, &my_locale_en_PH, &my_locale_en_ZA,
    &my_locale_en_ZW, &my_locale_es_AR, &my_locale_es_BO, &my_locale_es_CL,
    &my_locale_es_CO, &my_locale_es_CR, &my_locale_es_DO, &my_locale_es_EC,
    &my_locale_es_GT, &my_locale_es_HN, &my_locale_es_MX, &my_locale_es_NI,
    &my_locale_es_PA, &my_locale_es_PE, &my_locale_es_PR, &my_locale_es_PY,
    &my_locale_es_SV, &my_locale_es_US, &my_locale_es_UY, &my_locale_es_VE,
    &my_locale_fr_BE, &my_locale_fr_CA, &my_locale_fr_CH, &my_locale_fr_LU,
    &my_locale_it_IT, &my_locale_nl_BE, &my_locale_no_NO, &my_locale_sv_FI,
    &my_locale_zh_HK, &my_locale_el_GR, &my_locale_rm_CH, nullptr};

MY_LOCALE *my_locales_deprecated[] = {&my_locale_sr_YU, nullptr};

MY_LOCALE *my_locale_by_number(uint number) {
  MY_LOCALE *locale;
  if (number >= array_elements(my_locales) - 1) return nullptr;
  locale = my_locales[number];
  // Check that locale is on its correct position in the array
  DBUG_ASSERT(locale == my_locales[locale->number]);
  return locale;
}

static MY_LOCALE *my_locale_by_name(MY_LOCALE **locales, const char *name,
                                    size_t length) {
  MY_LOCALE **locale;
  for (locale = locales; *locale != nullptr; locale++) {
    if (length == strlen((*locale)->name) &&
        0 == native_strncasecmp((*locale)->name, name, length))
      return *locale;
  }
  return nullptr;
}

MY_LOCALE *my_locale_by_name(THD *thd, const char *name, size_t length) {
  MY_LOCALE *locale;

  if ((locale = my_locale_by_name(my_locales, name, length))) {
    // Check that locale is on its correct position in the array
    DBUG_ASSERT(locale == my_locales[locale->number]);
    return locale;
  } else if ((locale =
                  my_locale_by_name(my_locales_deprecated, name, length))) {
    /*
      Replace the deprecated locale to the corresponding
      'fresh' locale with the same ID.
    */
    locale = my_locales[locale->number];
    if (thd) {
      // Send a warning to the client
      push_warning_printf(
          thd, Sql_condition::SL_WARNING, ER_WARN_DEPRECATED_SYNTAX,
          ER_THD(thd, ER_WARN_DEPRECATED_SYNTAX), name, locale->name);
    } else {
      // Send a warning to mysqld error log
      LogErr(WARNING_LEVEL, ER_DEPRECATE_MSG_WITH_REPLACEMENT, name,
             locale->name);
    }
  }
  return locale;
}

void cleanup_errmsgs() {
  for (MY_LOCALE_ERRMSGS *msgs = global_errmsgs; msgs->get_language(); msgs++) {
    msgs->destroy();
  }
}
