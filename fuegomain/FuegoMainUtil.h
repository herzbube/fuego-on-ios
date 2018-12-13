//----------------------------------------------------------------------------
/** @file FuegoMainUtil.h */
//----------------------------------------------------------------------------

#ifndef FUEGOMAIN_UTIL_H
#define FUEGOMAIN_UTIL_H

#include <string>
#include "boost/filesystem/path.hpp"

class GoBook;
using boost::filesystem::path;

//----------------------------------------------------------------------------

namespace FuegoMainUtil
{

    /** Try to load opening book from a set of known paths.
        The file name is "book.dat". The paths tried are (in this order):
        - the directory of the executable
        - ABS_TOP_SRCDIR/book
        - DATADIR/PACKAGE
        @param book The opening book to load
        @param programDir the directory of the executable (may be a relative
        path or an empty string)
        @throws SgException, if book is not found */
    void LoadBook(GoBook& book, const path& programDir);

    /** Return Fuego version.
        If the macro VERSION was defined by the build system during compile
        time, its value is used as the version, otherwise the version
        is "(__DATE__)". 
        If SVNREV is defined, ( SVNREV ) is added.
        If compiled in debug mode, " (dbg)" is added. */
    std::string Version();

    /** Don't care whether Fuego is run from the command line, or invoked via
        function call. */
    int FuegoMain(int argc, char** argv);
    int FuegoMain(int argc, char** argv, std::istream* gtpClientInputStream, std::ostream* gtpClientOutputStream);
}

//----------------------------------------------------------------------------

#endif // FUEGOMAIN_UTIL_H
