<?php
/*
  +----------------------------------------------------------------------+
  | XHP                                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 2009 - 2012 Facebook, Inc. (http://www.facebook.com)   |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE.PHP, and is    |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
*/

/**
 * This is the base library of HTML elements for use in XHP. This includes all
 * non-deprecated tags and attributes. Elements in this file should stay as
 * close to spec as possible. Facebook-specific extensions should go into their
 * own elements.
 */
abstract class :xhp:html-element extends :x:primitive {

  // TODO: Break these out into abstract elements so that elements that need
  // them can steal the definition. Right now this is an overloaded list of
  // attributes.
  attribute
    // HTML attributes
    string accesskey, string class, bool contenteditable, string contextmenu,
    string dir, bool draggable, string dropzone, bool hidden, string id,
    string lang, bool spellcheck, string style, string tabindex, string title,
    enum {'yes', 'no'} translate,

    // Javascript events
    string onabort, string onblur, string oncanplay, string oncanplaythrough,
    string onchange, string onclick, string oncontextmenu, string oncuechange,
    string ondblclick, string ondrag, string ondragend, string ondragenter,
    string ondragleave, string ondragover, string ondragstart, string ondrop,
    string ondurationchange, string onemptied, string onended, string onerror,
    string onfocus, string oninput, string oninvalid, string onkeydown,
    string onkeypress, string onkeyup, string onload, string onloadeddata,
    string onloadedmetadata, string onloadstart, string onmousedown,
    string onmousemove, string onmouseout, string onmouseover, string onmouseup,
    string onmousewheel, string onpause, string onplay, string onplaying,
    string onprogress, string onratechange, string onreadystatechange,
    string onreset, string onscroll, string onseeked, string onseeking,
    string onselect, string onshow, string oninstalled, string onsubmit,
    string onsuspend, string ontimeupdate, string onvolumechange,
    string onwaiting,

    // IE only
    string onmouseenter, string onmouseleave;

  protected
    $tagName;

  public function getID() {
    return $this->requireUniqueId();
  }

  public function requireUniqueId() {
    if (!($id = $this->getAttribute('id'))) {
      $this->setAttribute('id', $id = substr(md5(mt_rand(0, 100000)), 0, 10));
    }
    return $id;
  }

  protected final function renderBaseAttrs() {
    $buf = '<'.$this->tagName;
    foreach ($this->getAttributes() as $key => $val) {
      if ($val !== null && $val !== false) {
        $buf .= ' ' . htmlspecialchars($key) . '="' . htmlspecialchars($val, ENT_QUOTES) . '"';
      }
    }
    return $buf;
  }

  public function addClass($class) {
    $this->setAttribute('class', trim($this->getAttribute('class').' '.$class));
    return $this;
  }

  public function conditionClass($cond, $class) {
    if ($cond) {
      $this->addClass($class);
    }
    return $this;
  }

  protected function stringify() {
    $buf = $this->renderBaseAttrs() . '>';
    foreach ($this->getChildren() as $child) {
      $buf .= :x:base::renderChild($child);
    }
    $buf .= '</'.$this->tagName.'>';
    return $buf;
  }
}

/**
 * Subclasses of :xhp:html-singleton may not contain children. When
 * rendered they will be in singleton (<img />, <br />) form.
 */
abstract class :xhp:html-singleton extends :xhp:html-element {
  children empty;

  protected function stringify() {
    return $this->renderBaseAttrs() . ' />';
  }
}

/**
 * Subclasses of :xhp:pseudo-singleton may contain exactly zero or one
 * children. When rendered they will be in full open\close form, no matter how
 * many children there are.
 */
abstract class :xhp:pseudo-singleton extends :xhp:html-element {
  children (pcdata)*;
}

/**
 * Below is a big wall of element definitions. These are the basic building
 * blocks of XHP pages.
 */
class :a extends :xhp:html-element {
  attribute
    string href, string hreflang, string media, string rel, string target,
    string type,
    // Legacy
    string name;
  category %flow, %phrase, %interactive;
  // Should not contain %interactive
  children (pcdata | %flow)*;
  protected $tagName = 'a';
}

class :abbr extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'abbr';
}

class :address extends :xhp:html-element {
  category %flow;
  // May not contain %heading, %sectioning, :header, :footer, or :address
  children (pcdata | %flow)*;
  protected $tagName = 'address';
}

class :area extends :xhp:html-singleton {
  attribute
    string alt, string coords, string href, bool hreflang, string media,
    string rel,
    enum {
      'circ', 'circle', 'default', 'poly', 'polygon', 'rect', 'rectangle'
    } shape, string target, string type;
  category %flow, %phrase;
  protected $tagName = 'area';
}

class :article extends :xhp:html-element {
  category %flow, %sectioning;
  children (pcdata | %flow)*;
  protected $tagName = 'article';
}

class :aside extends :xhp:html-element {
  category %flow, %sectioning;
  children (pcdata | %flow)*;
  protected $tagName = 'aside';
}

class :audio extends :xhp:html-element {
  attribute
    bool autoplay, bool controls, bool loop, string mediagroup, bool muted,
    enum {'none', 'metadata', 'auto'} preload, string src;
  category %flow, %phrase, %embedded, %interactive;
  children (:source*, :track*, (pcdata | %flow)*);
  protected $tagName = 'audio';
}

class :b extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'b';
}

class :base extends :xhp:html-singleton {
  attribute string href, string target;
  category %metadata;
  protected $tagName = 'base';
}

class :bdi extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'bdi';
}

class :bdo extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'bdo';
}

class :blockquote extends :xhp:html-element {
  attribute string cite;
  category %flow;
  children (pcdata | %flow)*;
  protected $tagName = 'blockquote';
}

class :body extends :xhp:html-element {
  attribute
    string onafterprint, string onbeforeprint, string onbeforeunload,
    string onblur, string onerror, string onfocus, string onhaschange,
    string onload, string onmessage, string onoffline, string ononline,
    string onpagehide, string onpageshow, string onpopstate, string onredo,
    string onresize, string onscroll, string onstorage, string onundo,
    string onunload;
  children (pcdata | %flow)*;
  protected $tagName = 'body';
}

class :br extends :xhp:html-singleton {
  category %flow, %phrase;
  protected $tagName = 'br';
}

class :button extends :xhp:html-element {
  attribute
    bool autofocus, bool disabled, string form, string formaction,
    string formenctype, enum {'get', 'post'} formmethod, bool formnovalidate,
    string formtarget, string name, enum {'submit', 'button', 'reset'} type,
    string value;
  category %flow, %phrase, %interactive;
  // Should not contain interactive
  children (pcdata | %phrase)*;
  protected $tagName = 'button';
}

class :caption extends :xhp:html-element {
  // Should not contain :table
  children (pcdata | %flow)*;
  protected $tagName = 'caption';
}

class :canvas extends :xhp:html-element {
  attribute int height, int width;
  category %flow, %phrase, %embedded;
  // Should not contain :table
  children (pcdata | %flow)*;
  protected $tagName = 'canvas';
}

class :cite extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'cite';
}

class :code extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'code';
}

class :col extends :xhp:html-singleton {
  attribute int span;
  protected $tagName = 'col';
}

class :colgroup extends :xhp:html-element {
  attribute int span;
  children (:col)*;
  protected $tagName = 'colgroup';
}

class :command extends :xhp:html-element {
  attribute
    bool checked, bool disabled, string icon, string label, string radiogroup,
    enum {'checkbox', 'command', 'radio'} type;
  category %metadata, %flow, %phrase;
  children (pcdata | %flow)*;
  protected $tagName = 'command';
}

class :datalist extends :xhp:html-element {
  category %flow, %phrase;
  children (:option* | %phrase*);
  protected $tagName = 'datalist';
}

class :dd extends :xhp:html-element {
  children (pcdata | %flow)*;
  protected $tagName = 'dd';
}

class :del extends :xhp:html-element {
  attribute string cite, string datetime;
  category %flow, %phrase;
  // transparent
  children (pcdata | %flow)*;
  protected $tagName = 'del';
}

class :details extends :xhp:html-element {
  attribute bool open;
  category %flow, %phrase, %interactive;
  children (:summary, %flow+);
  protected $tagName = 'details';
}

class :div extends :xhp:html-element {
  category %flow;
  children (pcdata | %flow)*;
  protected $tagName = 'div';
}

class :dfn extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'dfn';
}

class :dl extends :xhp:html-element {
  category %flow;
  children (:dt+, :dd+)*;
  protected $tagName = 'dl';
}

class :dt extends :xhp:html-element {
  children (pcdata | %flow)*;
  protected $tagName = 'dt';
}

class :em extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'em';
}

class :embed extends :xhp:html-element {
  attribute
    int height, string src, string type, int width,
    /**
     * The following attributes are Flash specific.
     * Most notable use: youtube video embedding
     */
    bool allowfullscreen, enum {'always', 'never'} allowscriptaccess,
    string wmode;

  category %flow, %phrase, %embedded, %interactive;
  children (pcdata | %phrase)*;
  protected $tagName = 'embed';
}

class :fieldset extends :xhp:html-element {
  attribute bool disabled, string form, string name;
  category %flow;
  children (:legend?, (pcdata | %flow)*);
  protected $tagName = 'fieldset';
}

class :figcaption extends :xhp:html-element {
  children (pcdata | %flow)*;
  protected $tagName = 'figcaption';
}

class :figure extends :xhp:html-element {
  category %flow;
  children ((:figcaption, %flow+) | (%flow+, :figcaption?));
  protected $tagName = 'figure';
}

class :footer extends :xhp:html-element {
  category %flow;
  children (pcdata | %flow)*;
  protected $tagName = 'footer';
}

class :form extends :xhp:html-element {
  attribute
    string action, string accept-charset, enum {'on', 'off'} autocomplete,
    string enctype, enum {'get', 'post'} method, string name, bool novalidate,
    string target;
  category %flow;
  // Should not contain :form
  children (pcdata | %flow)*;
  protected $tagName = 'form';
}

class :h1 extends :xhp:html-element {
  category %flow;
  children (pcdata | %phrase)*;
  protected $tagName = 'h1';
}

class :h2 extends :xhp:html-element {
  category %flow;
  children (pcdata | %phrase)*;
  protected $tagName = 'h2';
}

class :h3 extends :xhp:html-element {
  category %flow;
  children (pcdata | %phrase)*;
  protected $tagName = 'h3';
}

class :h4 extends :xhp:html-element {
  category %flow;
  children (pcdata | %phrase)*;
  protected $tagName = 'h4';
}

class :h5 extends :xhp:html-element {
  category %flow;
  children (pcdata | %phrase)*;
  protected $tagName = 'h5';
}

class :h6 extends :xhp:html-element {
  category %flow;
  children (pcdata | %phrase)*;
  protected $tagName = 'h6';
}

class :head extends :xhp:html-element {
  children (%metadata*);
  protected $tagName = 'head';
}

class :header extends :xhp:html-element {
  category %flow, %heading;
  children (pcdata | %flow)*;
  protected $tagName = 'header';
}

class :hgroup extends :xhp:html-element {
  category %flow, %heading;
  children (:h1 | :h2 | :h3 | :h4 | :h5 | :h6)+;
  protected $tagName = 'hgroup';
}

class :hr extends :xhp:html-singleton {
  category %flow;
  protected $tagName = 'hr';
}

class :html extends :xhp:html-element {
  attribute string manifest, string xmlns;
  children (:head, :body);
  protected $tagName = 'html';
}

class :i extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'i';
}

class :iframe extends :xhp:pseudo-singleton {
  attribute
    string name, int height, string sandbox, bool seamless, string src,
    string srcdoc, int width;
  category %flow, %phrase, %embedded, %interactive;
  protected $tagName = 'iframe';
}

class :img extends :xhp:html-singleton {
  attribute
    string alt, int height, bool ismap, string src, string usemap, int width;
  category %flow, %phrase;
  protected $tagName = 'img';
}

class :input extends :xhp:html-singleton {
  attribute
    string accept, string alt, enum {'on', 'off'} autocomplete, bool autofocus,
    bool checked, string dirname, bool disabled, string form, string formaction,
    string formenctype, enum {'get', 'post'} formmethod, bool formnovalidate,
    string formtarget, int height, string list, float max, int maxlength,
    float min, bool multiple, string name, string pattern, string placeholder,
    bool readonly, bool required, int size, string src, float step, enum {
      'hidden', 'text', 'search', 'tel', 'url', 'email', 'password', 'datetime',
      'date', 'month', 'week', 'time', 'datetime-local', 'number', 'range',
      'color', 'checkbox', 'radio', 'file', 'submit', 'image', 'reset', 'button'
    } type, string value, int width;
  category %flow, %phrase, %interactive;
  protected $tagName = 'input';
}

class :ins extends :xhp:html-element {
  attribute string cite, string datetime;
  category %flow, %phrase;
  children (pcdata | %flow)*;
  protected $tagName = 'ins';
}

class :keygen extends :xhp:html-singleton {
  attribute
    bool autofocus, string challenge, bool disabled, string form,
    string keytype, string name;
  category %flow, %phrase, %interactive;
  protected $tagName = 'keygen';
}

class :kbd extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'kbd';
}

class :label extends :xhp:html-element {
  attribute string for, string form;
  category %flow, %phrase, %interactive;
  // may not contain label
  children (pcdata | %phrase)*;
  protected $tagName = 'label';
}

class :legend extends :xhp:html-element {
  children (pcdata | %phrase)*;
  protected $tagName = 'legend';
}

class :li extends :xhp:html-element {
  children (pcdata | %flow)*;
  protected $tagName = 'li';
}

class :link extends :xhp:html-singleton {
  attribute
    string href, string hreflang, string media, string rel @required,
    string sizes, string type;
  category %metadata;
  protected $tagName = 'link';
}

class :map extends :xhp:html-element {
  attribute string name;
  category %flow, %phrase;
  children (pcdata | %flow)*;
  protected $tagName = 'map';
}

class :mark extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'mark';
}

class :menu extends :xhp:html-element {
  attribute string label, enum {'toolbar', 'context'} type;
  category %flow, %interactive;
  children (:li* | %flow*);
  protected $tagName = 'menu';
}

class :meta extends :xhp:html-singleton {
  attribute
    // The correct definition of http-equiv is an enum, but there are legacy
    // values still used and any strictness here would only be frustrating.
    string charset, string content @required, string http-equiv, string name,
    // Facebook OG
    string property;
  category %metadata;
  protected $tagName = 'meta';
}

class :meter extends :xhp:html-element {
  attribute
    string form, float high, float low, float max, float min, float optimum,
    float value;
  category %flow, %phrase;
  // Should not contain :meter
  children (pcdata | %phrase)*;
  protected $tagName = 'meter';
}

class :nav extends :xhp:html-element {
  category %flow;
  children (pcdata | %flow)*;
  protected $tagName = 'nav';
}

class :noscript extends :xhp:html-element {
  children (pcdata)*;
  category %flow, %phrase, %metadata;
  protected $tagName = 'noscript';
}

class :object extends :xhp:html-element {
  attribute
    string data, int height, string form, string name, string type,
    string usemap, int width;
  category %flow, %phrase, %embedded, %interactive;
  children (:param*, (pcdata | %flow)*);
  protected $tagName = 'object';
}

class :ol extends :xhp:html-element {
  attribute bool reversed, int start, enum {'1', 'a', 'A', 'i', 'I'} type;
  category %flow;
  children (:li)*;
  protected $tagName = 'ol';
}

class :optgroup extends :xhp:html-element {
  attribute bool disabled, string label;
  children (:option)*;
  protected $tagName = 'optgroup';
}

class :option extends :xhp:pseudo-singleton {
  attribute bool disabled, string label, bool selected, string value;
  protected $tagName = 'option';
}

class :output extends :xhp:html-element {
  attribute string for, string form, string name;
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'output';
}

class :p extends :xhp:html-element {
  category %flow;
  children (pcdata | %phrase)*;
  protected $tagName = 'p';
}

class :param extends :xhp:pseudo-singleton {
  attribute string name, string value;
  protected $tagName = 'param';
}

class :pre extends :xhp:html-element {
  category %flow;
  children (pcdata | %phrase)*;
  protected $tagName = 'pre';
}

class :progress extends :xhp:html-element {
  attribute string form, float max, float value;
  category %flow, %phrase;
  // Should not contain :progress
  children (pcdata | %phrase)*;
  protected $tagName = 'progress';
}

class :q extends :xhp:html-element {
  attribute string cite;
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'q';
}

class :rp extends :xhp:html-element {
  children (pcdata | %phrase)+;
  protected $tagName = 'rp';
}

class :rt extends :xhp:html-element {
  children (pcdata | %phrase)+;
  protected $tagName = 'rt';
}

class :ruby extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata+, (:rt | (:rp, :rt, :rp)))+;
  protected $tagName = 'ruby';
}

class :s extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 's';
}

class :samp extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'samp';
}

class :script extends :xhp:pseudo-singleton {
  attribute bool async, string charset, bool defer, string src, string type,
  // Legacy
  string language;
  category %flow, %phrase, %metadata;
  protected $tagName = 'script';
}

class :section extends :xhp:html-element {
  category %flow, %sectioning;
  children (pcdata | %flow)*;
  protected $tagName = 'section';
}

class :select extends :xhp:html-element {
  attribute
    bool autofocus, bool disabled, string form, bool multiple, string name,
    bool required, int size;
  category %flow, %phrase, %interactive;
  children (:option | :optgroup)*;
  protected $tagName = 'select';
}

class :small extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'small';
}

class :source extends :xhp:html-singleton {
  attribute string media, string src, string type;
  protected $tagName = 'source';
}

class :span extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'span';
}

class :strong extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'strong';
}

class :style extends :xhp:pseudo-singleton {
  attribute
    enum {
      'all', 'aural', 'braille', 'embossed', 'handheld', 'print', 'projection',
      'screen', 'speech', 'tty', 'tv'
    } media, bool scoped, string type;
  category %flow, %metadata;
  protected $tagName = 'style';
}

class :sub extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'sub';
}

class :summary extends :xhp:html-element {
  children (pcdata | %phrase)*;
  protected $tagName = 'summary';
}

class :sup extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'sup';
}

class :table extends :xhp:html-element {
  attribute int border;
  category %flow;
  children (
    :caption?, :colgroup*,
    :thead?,
    (
      (:tfoot, (:tbody+ | :tr*)) |
      ((:tbody+ | :tr*), :tfoot?)
    )
  );
  protected $tagName = 'table';
}

class :tbody extends :xhp:html-element {
  children (:tr)*;
  protected $tagName = 'tbody';
}


class :td extends :xhp:html-element {
  attribute int colspan, string headers, int rowspan;
  children (pcdata | %flow)*;
  protected $tagName = 'td';
}

class :textarea extends :xhp:pseudo-singleton {
  attribute
    bool autofocus, int cols, string dirname, bool disabled, string form,
    int maxlength, string name, string placeholder, bool readonly,
    bool required, int rows, enum {'soft', 'hard'} wrap;
  category %flow, %phrase, %interactive;
  protected $tagName = 'textarea';
}

class :tfoot extends :xhp:html-element {
  children (:tr)*;
  protected $tagName = 'tfoot';
}

class :th extends :xhp:html-element {
  attribute
    int colspan, string headers, int rowspan,
    enum {'col', 'colgroup', 'row', 'rowgroup'} scope;
  children (pcdata | %flow)*;
  protected $tagName = 'th';
}

class :thead extends :xhp:html-element {
  children (:tr)*;
  protected $tagName = 'thead';
}

class :time extends :xhp:html-element {
  attribute string datetime, bool pubdate;
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'time';
}

class :title extends :xhp:pseudo-singleton {
  category %metadata;
  protected $tagName = 'title';
}

class :tr extends :xhp:html-element {
  children (:th | :td)*;
  protected $tagName = 'tr';
}

class :track extends :xhp:html-singleton {
  attribute
    bool default,
    enum {'subtitles', 'captions', 'descriptions', 'chapters', 'metadata'} kind,
    string label, string src, string srclang;
  protected $tagName = 'track';
}

class :tt extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'tt';
}

class :u extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'u';
}

class :ul extends :xhp:html-element {
  category %flow;
  children (:li)*;
  protected $tagName = 'ul';
}

class :var extends :xhp:html-element {
  category %flow, %phrase;
  children (pcdata | %phrase)*;
  protected $tagName = 'var';
}

class :video extends :xhp:html-element {
  attribute
    bool autoplay, bool controls, int height, bool loop, string mediagroup,
    bool muted, string poster, enum {'none', 'metadata', 'auto'} preload,
    string src, int width;
  category %flow, %phrase, %embedded, %interactive;
  children (:source*, :track*, (pcdata | %flow)*);
  protected $tagName = 'video';
}

class :wbr extends :xhp:html-singleton {
  category %flow, %phrase;
  protected $tagName = 'wbr';
}

/**
 * Render an <html /> element with a DOCTYPE, XHP has chosen to only support
 * the HTML5 doctype.
 */
class :x:doctype extends :x:primitive {
  children (:html);

  protected function stringify() {
    $children = $this->getChildren();
    return '<!DOCTYPE html>' . (:x:base::renderChild($children[0]));
  }
}
