//
// $Id: basic_location.hh,v 1.6 2004/03/03 12:56:18 cholm Exp $ 
//  
//  basic_location.hh
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
#ifndef YLMM_basic_location
#define YLMM_basic_location
#ifndef __IOSTREAM__
# include <iostream>
#endif
/**  @file   ylmm/basic_location.hh
     @author Christian Holm Christensen
     @date   Mon Sep 30 00:08:22 2002
     @brief  Declaration of parser basic_location class. */

namespace ylmm 
{
  /** @class basic_location basic_location.hh <ylmm/basic_location.hh>
      @brief Class to handle basic location inforamtion.  

      This class embeds information on where in a parse stream the
      parser encountered what tokens.  The class defines a region
      starting at a line number, column number pair, extending until
      a line number, column number pair.  If users want to, they can
      subclass this class to allow it to provide additional
      information. 
      @ingroup yacc
  */ 
  class basic_location 
  {
  protected:
    int _first_line;		/** The first line of the current
				    region */ 
    int _first_column;		/** The first column of the current
				    region */
    int _last_line;		/** The last line of the current
				    region */
    int _last_column;		/** The last column of the current
				    region */
  public:
    /** Make a new basic_location.  
	Set the region to be at the arguments. 
	@param l1 new first line 
	@param c1 new first column 
	@param l2 new last line 
	@param c2 new last column. */	
    basic_location(int l1=0, int c1=0, int l2=0, int c2=0); 
    /** Constructor.  Make a new basic_location as a region of two
	other locations. 
	@param f The position of the first line/column, taken from the
	first line/column of @a f
	@param l The position of the last line/column, taken from the
	last line/column of @a l */
    basic_location(const basic_location& f, const basic_location& l) 
    {
      region(f, l);
    }
    /** Make a copy of a basic_location. 
	@param l the basic_location to copy. */ 
    basic_location(const basic_location& l) { copy(l); }
    /** Do-nothing dtor. */
    virtual ~basic_location() {}

    //@{
    /// @name Copying and the like
    /** Assign a basic_location from another basic_location. 
	@param l basic_location to copy. 
	@return this object */
    virtual basic_location& operator=(const basic_location& l) { 
      copy(l); return *this; }
    /** Make a copy of a basic_location. 
	@param l the basic_location to copy. */ 
    virtual void copy(const basic_location& l);
    /** Make a region from two basic_locations. 
	@param l1 the beginning of the region. 
	@param l2 the end of the region. */
    virtual void region(const basic_location& l1, const basic_location& l2);
    /// Set start of region from anoher basic_location. 
    void first(const basic_location& l);
    /// Set end of region from anoher basic_location. 
    void last(const basic_location& l);
    //@}

    //@{
    /// @name Get the fields
    /** Get the first line of the region. 
        @return  the first line of the region.  */
    int  first_line()   const { return _first_line; } 
    /** Get the first column of the region. 
        @return  the first column of the region.  */
    int  first_column() const { return _first_column; }
    /** Get the last line of the region. 
        @return  the last line of the region.  */
    int  last_line()    const { return _last_line; }
    /** Get the last column of the region. 
        @return  the last column of the region.  */
    int  last_column()  const { return _last_column; }
    //@}

    //@{
    /// Get the fields
    /** Set start of region. */
    void first(int line, int column); 
    /** Set first line of the region. */
    int  first_line(int first_line)     { return _first_line   =first_line; }
    /** Set first column of the region. */
    int  first_column(int first_column) { return _first_column =first_column; }
    /** Set end of region. */
    void last(int line, int column);
    /** Set last line of the region. */
    int  last_line(int last_line)       { return _last_line    = last_line; }
    /** Set last column of the region. */
    int  last_column(int last_column)   { return _last_column  = last_column; }
    //@}
  };

  //__________________________________________________________________
  inline basic_location::basic_location(int first_line, int first_column, 
					int last_line, int last_column) 
  {
    first(first_line, first_column); 
    last(last_line, last_column); 
  }

  //__________________________________________________________________
  inline void basic_location::copy(const basic_location& l) 
  {
    _first_line = l._first_line; _first_column = l._first_column; 
    _last_line  = l._last_line;  _last_column  = l._last_column; 
  }
  
  //__________________________________________________________________
  inline void basic_location::region(const basic_location& l1, 
				     const basic_location& l2) 
  { 
    first(l1); last(l2); 
  }
  
  //__________________________________________________________________
  inline void basic_location::first(int line, int column) 
  {
    _first_line = line; _first_column = column; 
  }
  
  //__________________________________________________________________
  inline void basic_location::first(const basic_location& l) 
  {
    first(l._first_line, l._first_column);
  }

  //__________________________________________________________________
  inline void basic_location::last(int line, int column) 
  {
    _last_line = line; _last_column = column; 
  }
  
  //__________________________________________________________________
  inline void basic_location::last(const basic_location& l) 
  {
    last(l._last_line, l._last_column);
  }
}

//____________________________________________________________________
/** Output a ylmm::basic_location object to a stream
    @param o Output stream 
    @param l the ylmm::basic_location to show
    @return @a o */
inline std::ostream& 
operator<<(std::ostream& o, const ylmm::basic_location& l) 
{
  return o << l.first_line() << "," << l.first_column() 
	   << ":" << l.last_line() << "," << l.last_column();
}


#endif 
//____________________________________________________________________
//
// EOF
//
