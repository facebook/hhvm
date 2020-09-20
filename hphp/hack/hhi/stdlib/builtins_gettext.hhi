<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the 'hack' directory of this source tree.
 *
 */

<<__PHPStdLib>>
function bind_textdomain_codeset(string $domain, string $codeset);
<<__PHPStdLib>>
function bindtextdomain(string $domain_name, string $dir);
<<__PHPStdLib>>
function dcgettext(string $domain_name, string $msgid, int $category);
<<__PHPStdLib>>
function dcngettext(string $domain, string $msgid1, string $msgid2, int $count, int $category);
<<__PHPStdLib>>
function dgettext(string $domain_name, string $msgid);
<<__PHPStdLib>>
function dngettext(string $domain, string $msgid1, string $msgid2, int $count);
<<__PHPStdLib>>
function gettext(string $msgid);
<<__PHPStdLib>>
function ngettext(string $msgid1, string $msgid2, int $count);
<<__PHPStdLib>>
function textdomain(string $domain);
