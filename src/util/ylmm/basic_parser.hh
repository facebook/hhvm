//
// $Id: basic_parser.hh,v 1.15 2005/03/09 17:19:43 cholm Exp $
//
//  basic_parser.hh
//  Copyright (C) 2002 Christian Holm Christensen <cholm@nbi.dk>
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public License
//  as published by the Free Software Foundation; either version 2.1
//  of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free
//  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//  02111-1307 USA
//
#ifndef YLMM_basic_parser
#define YLMM_basic_parser
#ifndef YLMM_basic_messenger
# include <util/ylmm/basic_messenger.hh>
#endif
#ifndef YLMM_basic_location
# include <util/ylmm/basic_location.hh>
#endif

/**  @file   basic_parser.hh
     @author Christian Holm Christensen
     @date   Mon Sep 30 00:08:22 2002
     @brief  Declaration of parser ABC. */

/** Namespace for all parser and scanner classes. */
namespace ylmm
{
  //=====================================================================
  /** @class parser_base basic_parser.hh <ylmm/basic_parser.hh>
      @brief ABC for parser classes.

      This defines the common interface for parser classes, that
      doesn't depend on the semantic and location type.  It's
      interface can safely be used in the application or via some
      library.
      @param lock The type of thread syncronisation locks used in the
      application.  Per default a single threaded locking mechanism
      (that is - no locks) is used. The application should pass the
      appropiate type here to make the use of this class thread safe.
      @ingroup yacc
  */
  template<typename Lock=basic_lock>
  class parser_base
  {
  public:
    /** Type of thread locks  */
    typedef Lock lock_type;
  protected:
    basic_messenger<lock_type>* _messenger; /** The output handler */
    /** Do-nothing CTOR. */
    parser_base();
    /** Do-nothing DTOR. */
    virtual ~parser_base() {}
  public:
    //@{
    /// @name Abstract interface
    /** Get whether the parser is tracing or not.
	This member function is implmented via yaccmm.hh.
	@return true if we're tracing the parse */
    virtual bool tracing() const = 0;
    /** Set whether the parser is tracing or not.
	This member function is implmented via yaccmm.hh.
	@param t if true, trace the parse
	@return true if we're tracing the parse */
    virtual bool tracing(bool t) = 0;
    /** The main function member.
	@param arg Optional parameter
	@return 0 on success, non-zero on failure */
    virtual int  parse(void* arg=0) = 0;
    /** Input member function.
	@param arg optinal argument.
	@return next token ID number */
    virtual int  scan(void* arg=0) = 0;
    //@}

    //@{
    /// @name Output handler
    /** Get the output handler
	@return The output handler used by this parser. */
    virtual basic_messenger<lock_type>& messenger() const;
    /** Set and get the output handler
	@param o The output handler to use.
	@return The output handler used by this parser. */
    virtual
    basic_messenger<lock_type>& messenger(basic_messenger<lock_type>& o);
    //@}

    //@{
    /// @name message handling
    /** Called on errors.
	When the generated parser detects a parse error, this member
	function is called.  Per default, it simply outputs the error
	message to std::cerr, but user code can override this to do
	fallbacks, etc.
	@param msg Is the message from the parser. */
    virtual void fatal(const char* msg);
    /** Error printing - service function.
	When the generated parser wants to output error information,
	this is the member function that will be invoked. Per default
	it outputs the message to standard error.
	@param message The message to print */
    virtual void error(const char* message, ...);
    /** Message printing - service function.  When the generated
	parser wants to output various information, this is the member
	function that will be invoked. Per default it outputs the
	message to standard output.  Also, when the generated parser
	wants to output trace information, this is the member function
	that will be invoked. Per default it outputs the message to
	standard error.  @param text The message to print */
    virtual void message(const char* text, ...);
    //@}
  };
  //__________________________________________________________________
  template <typename L>
  inline
  parser_base<L>::parser_base()
  {
    _messenger = basic_messenger<L>::default_handler();
  }
  //__________________________________________________________________
  template <typename L>
  inline basic_messenger<L>& parser_base<L>::messenger() const
  {
    return *_messenger;
  }

  //__________________________________________________________________
  template <typename L>
  inline basic_messenger<L>&
  parser_base<L>::messenger(basic_messenger<lock_type>& output)
  {
    _messenger = &output;
    return *_messenger;
  }

  //__________________________________________________________________
  template <typename L>
  inline void parser_base<L>::error(const char* message, ...)
  {
    if (!_messenger) return;
    va_list ap;
    va_start(ap, message);
    _messenger->error(message, ap);
    va_end(ap);
  }
  //__________________________________________________________________
  template <typename L>
  inline void parser_base<L>::message(const char* message, ...)
  {
    if (!_messenger) return;
    va_list ap;
    va_start(ap, message);
    _messenger->message(message, ap);
    va_end(ap);
  }
  //__________________________________________________________________
  template <typename L>
  inline void parser_base<L>::fatal(const char* message)
  {
    if (!_messenger) return;
    _messenger->error(message);
  }

  //=====================================================================
  /** @class basic_parser basic_parser.hh <ylmm/basic_parser.hh>
      @brief Basic parser implementation.

      This templated class implements a parser, or rather, a specific
      instantation of this tempalte implmentes it.  To use it, define
      the the processor macro @c YLMM_PARSER_CLASS to a specific
      instance of this template, or a sub-class ofa specific instance
      in your @b Yacc input file to.  Note that the third argument,
      @p id can be used to disambiqutate two parsers with the same
      @p Token,  @p Location pair.
      See also @ref parser_doc.
      @param Token the type of the tokens used in the grammar.
      @param Location the location type used, if applicable for the
      backend parser generator (Normal @b Yacc does not support that,
      but @b Bison does).
      @param id is an integer that uniquely identifies the class.
      @param lock The type of thread syncronisation locks used in the
      application.  Per default a single threaded locking mechanism
      (that is - no locks) is used. The application should pass the
      appropiate type here to make the use of this class thread safe.
      @ingroup yacc
  */
  template <typename Token, typename Location=basic_location,
	    int id=0, typename Lock=basic_lock>
  class basic_parser : public parser_base<Lock>
  {
  public:
    /// Type of tokens
    typedef Token token_type;
    /// Type of locationss
    typedef Location location_type;
    /// The Id of the parser class
    enum { parser_id = id };
  protected:
    typedef parser_base<Lock> base_type;
    token_type* _token;		/** Address of the current token */
    location_type* _location;	/** The current location. */
  public:
    /** Do-nothing CTOR. */
    basic_parser() : _token(0), _location(0) {}
    /** Do-nothing DTOR. */
    virtual ~basic_parser() { }

    //@{
    /// @name Debug handling
    /** Get whether the parser is tracing or not.
	This member function is implmented via yaccmm.hh.
	@return true if we're tracing the parse */
    virtual bool tracing() const;
    /** Set whether the parser is tracing or not.
	This member function is implmented via yaccmm.hh.
	@param t if true, trace the parse
	@return true if we're tracing the parse */
    virtual bool tracing(bool t);
    //@}

    //@{
    /// @name Location information
    /** Test if we need location information.
	@return true if we need location information */
    virtual bool need_where() const { return _location != 0; }
    /** Get the location of next token to be parsed.
	@return  The location of next token to be parsed. */
    virtual location_type& where() { return *_location; }
    /** Set the location of next token to be parsed.
	Must be invoked from the the scanner.
	@param loc The location of the next token to be parsed.
	@return  The location of next token to be parsed. */
    virtual location_type& where(location_type& loc);
    /** Set the address of the location information
	@param loc the address of the location information  */
    void where_addr(location_type* loc) { _location = loc; }
    //@}

    //@{
    /// @name Token handling
    /** Get the next token to be parsed.
	@return The next token to be parsed. */
    virtual token_type& token() const { return *_token; }
    /** Set the next token to be parsed.
	Must be invoked from the the scanner.
	@param token The next token to be parsed.
	@return The next token to be parsed. */
    virtual token_type& token(token_type& token);
    /** Set the address of next token to be parsed.
	@param token The next address of the token to be parsed. */
    void token_addr(token_type* token) { _token = token; }
    //@}

    //@{
    /// @name Scanning and parsing
    /** The main function member.
	This member function starts the parser.  It should call the C
	function generated by Yacc.  This member function is
	implmented via yaccmm.hh.
	@param arg user defined data
	@return return value of yyparse. */
    virtual int  parse(void* arg=0);
    /** Input member function.
	When the parser wants more input, it calls this member
	function to get one (or more) token(s).  The member function
	must return the token id value, as well as set the current
	token (using the token member function) and possibly
	location information (using the location member
	function).  One way of implementing this, is to use a @b Lex
	generated scanner with the ylmm::basic_scanner class, and call
	the member function ylmm::basic_scanner::next, passing it the
	proper arguments.
	@param arg A pointer to user defined data.
	@return token ID number. */
    virtual int  scan(void* arg=0) = 0;
    //@}

    //@{
    /// @name Message handling
    /** Print info on currently passed token.
	When tracing is enabled, then this member function is
	invoked when a message about a token is to be output.  Per
	default it writes the lookahead number and the address of the
	token.
	@param lookahead The current lookahead.
	@param token The current token. */
    virtual void trace(int lookahead, const token_type& token);
    //@}
  };
  //__________________________________________________________________
  template <typename T, typename L, int id, typename M>
  inline L&
  basic_parser<T,L,id,M>::where(location_type& loc)
  {
    *_location = loc;
    return *_location;
  }
  //__________________________________________________________________
  template <typename T, typename L, int id, typename M>
  inline T&
  basic_parser<T,L,id,M>::token(token_type& token)
  {
    *_token = token;
    return *_token;
  }
  //__________________________________________________________________
  template <typename T, typename L, int id, typename M>
  inline void
  basic_parser<T,L,id,M>::trace(int lookahead,
				const token_type& token)
  {
    if (!base_type::_messenger) return;
    base_type::_messenger->message_stream() << " " << lookahead
					    << " [" << token << "]"
					    << std::flush;
  }
}


#endif
//____________________________________________________________________
//
// EOF
//
