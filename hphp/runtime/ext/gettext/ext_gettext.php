<?hh

/* With bind_textdomain_codeset(), you can set in which encoding will be
 * messages from domain returned by gettext() and similar functions.
 */
<<__Native>>
function bind_textdomain_codeset(string $domain,
                                 string $codeset): mixed;

/* The bindtextdomain() function sets the path for a domain.
 */
<<__Native>>
function bindtextdomain(string $domain,
                        string $directory): mixed;

/* This function allows you to override the current domain for a single
 * message lookup.
 */
<<__Native>>
function dcgettext(string $domain,
                   string $message,
                   int $category): mixed;

/* This function allows you to override the current domain for a single plural
 * message lookup.
 */
<<__Native>>
function dcngettext(string $domain,
                    string $msgid1,
                    string $msgid2,
                    int $n,
                    int $category): mixed;

/* The dgettext() function allows you to override the current domain for a
 * single message lookup.
 */
<<__Native>>
function dgettext(string $domain,
                  string $message): mixed;

/* The dngettext() function allows you to override the current domain for a
 * single plural message lookup.
 */
<<__Native>>
function dngettext(string $domain,
                   string $msgid1,
                   string $msgid2,
                   int $n): mixed;

/* Looks up a message in the current domain.
 */
<<__Native>>
function gettext(string $message): mixed;

/* An alias for gettext.
 */
<<__Native>>
function _(string $message): mixed;

/* The plural version of gettext(). Some languages have more than one form for
 * plural messages dependent on the count.
 */
<<__Native>>
function ngettext(string $msgid1,
                  string $msgid2,
                  int $n): mixed;

/* This function sets the domain to search within when calls are made to
 * gettext(), usually the named after an application.
 */
<<__Native>>
function textdomain(string $text_domain): mixed;
