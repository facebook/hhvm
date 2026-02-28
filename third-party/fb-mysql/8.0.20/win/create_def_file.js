// create_def_file.js
//
// Copyright (c) 2009, 2017, Oracle and/or its affiliates. All rights reserved.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is also distributed with certain software (including
// but not limited to OpenSSL) that is licensed under separate terms,
// as designated in a particular file or component or in included license
// documentation.  The authors of MySQL hereby grant you an additional
// permission to link the program and your derivative works with the
// separately licensed software that they have included with MySQL.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

/*
  This script extracts names and types of globally defined symbols from
  COFF object files, and writes this information to stdout using .DEF 
  file format (module definition file used by Microsoft linker)
  
  In MySQL this script is used to export symbols from mysqld.exe for use by
  storage engine DLLs.
  
  Usage:
  cscript create_def_file.js [x86|x64] [object|static_lib|directory ...]
  
  If directory is passed as a parameter, script will process all object files
  and static libraries in this directory and recursively in all subdrectories.

  Note :The script does not work properly if  /GL (global optimization) 
  compiler option was used to produce object files or libraries. This is a
  limitation of the dumpbin tool that is used by the script.
*/

ForReading = 1;
ForWriting = 2;
ForAppending = 8;


var args = WScript.Arguments;
  
// check that we got proper arguments
if (args.length < 2)
{
    echo("Usage: create_def_file <X86|X64> [static_library|objectfile|objectdirectory ...] ");
    WScript.Quit(1);
}


var is64 = args.Item(0).toLowerCase() == "x64";
var shell = new ActiveXObject("WScript.Shell");
var fso = new ActiveXObject("Scripting.FileSystemObject");

OutputSymbols(CollectSymbols());


// takes the array that has been built up and writes out mysqld.def
function OutputSymbols(symbols)
{
    var out = WScript.StdOut;
    out.WriteLine("EXPORTS");
    for (var sym in symbols)
       out.WriteLine(sym);
}

function echo(message)
{
    WScript.StdErr.WriteLine(message);
}

// Extract global symbol names and type from objects
function CollectSymbols()
{
    var uniqueSymbols = new Array();

    try
    {
        /*
         Compiler tools use VS_UNICODE_OUTPUT env. variable as indicator 
         that they run within IDE, so they can communicate with IDE via 
         pipes instead of usual stdout/stderr. Refer to 
         http://blogs.msdn.com/freik/archive/2006/04/05/569025.aspx 
         for more info.
         Unset this environment variable.
        */
        shell.Environment("PROCESS").Remove("VS_UNICODE_OUTPUT");
    }
    catch(e){}
 
    var rspfilename = "dumpsymbols.rsp";
    CreateResponseFile(rspfilename);
    var commandline="dumpbin @"+rspfilename;
    
    echo("Executing "+commandline);
    var oExec = shell.Exec(commandline);

    while(!oExec.StdOut.AtEndOfStream)
    {
        var line = oExec.StdOut.ReadLine();
        if (line.indexOf("External") == -1) continue;
        var columns = line.split(" ");
        var index = 0;
        if (columns.length < 3)
          continue;

        /*
          If the third column of dumpbin output contains SECTx, 
          the symbol is defined in that section of the object file. 
          If UNDEF appears, it is not defined in that object and must
          be resolved elsewhere. BSS symbols (like uninitialized arrays)
          appear to have non-zero second column.
        */
        if (columns[2].substring(0,4)!="SECT")
        {
          if (columns[2] == "UNDEF"  && parseInt(columns[1])==0 )
            continue;
        }

        /*
          Extract undecorated symbol names 
          between "|" and next whitespace after it.
        */
        for (; index < columns.length; index++)
          if (columns[index] == "|")
              break;

        var symbol = columns[index + 1];
        var firstSpace = symbol.indexOf(" ");
        if (firstSpace != -1)
            symbol = symbol.substring(0, firstSpace-1);

        // Don't export compiler defined stuff
        if (IsCompilerDefinedSymbol(symbol))
            continue;
        
        // Correct symbol name for cdecl calling convention on x86
        symbol = ScrubSymbol(symbol);

        // Check if we have function or data
        if (line.indexOf("notype () ") == -1)
            symbol = symbol + " DATA";

        uniqueSymbols[symbol] = 1;
    }
    fso.DeleteFile(rspfilename);
    return uniqueSymbols;
}

// performs necessary cleanup on the symbol name
function ScrubSymbol(symbol)
{
    if (is64) return symbol;
    if (symbol.charAt(0) != "_") 
        return symbol;

    var atSign = symbol.indexOf("@");
    if (atSign != -1)
    {
        var paramSize = symbol.substring(atSign+1, symbol.Length);
        if (paramSize.match("[0-9]+$")) return symbol;
    }
    return symbol.substring(1, symbol.length);
}

// returns true if the symbol is compiler defined
function IsCompilerDefinedSymbol(symbol)
{
    return ((symbol.indexOf("__real@") != -1) ||
    (symbol.indexOf("__xmm@") != -1) ||        // SSE instruction set constants
    (symbol.indexOf("_CTA2?") != -1) ||        // std::bad_alloc
    (symbol.indexOf("_CTA3?") != -1) ||        // std::length_error
    (symbol.indexOf("_CTA4?") != -1) ||        // std::ios_base::failure
    (symbol.indexOf("_CTA5?") != -1) ||        // std::ios_base::failure
    (symbol.indexOf("_CTA6?") != -1) ||        // boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::bad_get>>
    (symbol.indexOf("_CTA7?") != -1) ||        // boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::bad_lexical_cast>>
    (symbol.indexOf("_CTA8?AV?") != -1) ||     //         bad_rational
    (symbol.indexOf("_TI2?") != -1) ||         // std::bad_alloc
    (symbol.indexOf("_TI3?") != -1) ||         // std::length_error
    (symbol.indexOf("_TI4?") != -1) ||         // std::ios_base::failure
    (symbol.indexOf("_TI5?") != -1) ||         // std::ios_base::failure
    (symbol.indexOf("_TI6?") != -1) ||         // boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::bad_get>>
    (symbol.indexOf("_TI7?") != -1) ||         // boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::bad_lexical_cast>>
    (symbol.indexOf("_TI8?AV?") != -1) ||      //         bad_rational
    (symbol.indexOf("_RTC_") != -1) || 
    (symbol.indexOf("??_C@_") != -1) ||
    (symbol.indexOf("??_R") != -1) ||
    (symbol.indexOf("??_7") != -1)  ||
    (symbol.indexOf("?_G") != -1) ||           // scalar deleting destructor
    (symbol.indexOf("_VInfreq_?") != -1) ||    // special label (exception handler?) for Intel compiler
    (symbol.indexOf("?_E") != -1));            // vector deleting destructor
}

// Creates response file for dumpbin
function CreateResponseFile(filename)
{
  var responseFile = fso.CreateTextFile(filename,true);
  responseFile.WriteLine("/SYMBOLS");
  
  var index = 1;
  for (; index < args.length; index++)
  {
    addToResponseFile(args.Item(index),responseFile);
  }
  responseFile.Close();
}

// Add object file/library to the dumpbin response file.
// If filename parameter is directory, all objects and libs under 
// this directory or subdirectories are added.
function addToResponseFile(filename, responseFile)
{
   if (fso.FolderExists(filename))
   {
        var folder = fso.getFolder(filename);
        var enumerator = new Enumerator(folder.files);
        for (; !enumerator.atEnd(); enumerator.moveNext())
        {
            addToResponseFile(enumerator.item().Path, responseFile);
        }
        enumerator = new Enumerator(folder.subFolders);
        for (; !enumerator.atEnd(); enumerator.moveNext())
        {
            addToResponseFile(enumerator.item().Path, responseFile);
        }
    }
    else if (fso.FileExists(filename))
    {
       var extension = filename.substr(filename.length -3).toLowerCase();
       if(extension == "lib" || extension == "obj")
       {
           responseFile.WriteLine("\""+fso.GetFile(filename).Path+"\"");
       }
    }
}
