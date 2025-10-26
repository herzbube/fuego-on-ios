//----------------------------------------------------------------------------
/** @file FuegoMainUtil.cpp
    See FuegoMainUtil.h */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "FuegoMainUtil.h"

#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>
#include "GoBook.h"
#include "SgDebug.h"
#include "SgStringUtil.h"

//----------------------------------------------------------------------------

namespace {

bool LoadBookFile(GoBook& book, const boost::filesystem::path& file)
{
    std::string nativeFile = SgStringUtil::GetNativeFileName(file);
    SgDebug() << "Loading opening book from '" << nativeFile << "'... ";
    std::ifstream in(nativeFile.c_str());
    if (! in)
    {
        SgDebug() << "not found\n";
        return false;
    }
    try
    {
        book.Read(in);
    }
    catch (const SgException& e)
    {
        SgDebug() << "error: " << e.what() << '\n';
        return false;
    }
    SgDebug() << "ok\n";
    return true;
}

} // namespace

//----------------------------------------------------------------------------

void FuegoMainUtil::LoadBook(GoBook& book,
                             const boost::filesystem::path& programDir)
{
    const std::string fileName = "book.dat";
    using boost::filesystem::path;
    #ifdef ABS_TOP_SRCDIR
        if (LoadBookFile(book, path(ABS_TOP_SRCDIR) / "book" / fileName))
            return;
    #endif
    if (LoadBookFile(book, programDir / fileName))
        return;
    #if defined(DATADIR) && defined(PACKAGE)
        if (LoadBookFile(book, path(DATADIR) / PACKAGE / fileName))
            return;
    #endif
    throw SgException("Could not find opening book.");
}

std::string FuegoMainUtil::Version()
{
    std::ostringstream s;
#ifdef VERSION
    s << VERSION;
#else
    s << "(" __DATE__ ")";
#endif
#ifdef SVNREV
    s << "(" SVNREV ")";
#endif
#ifndef NDEBUG
    s << " (dbg)";
#endif
    return s.str();
}

//----------------------------------------------------------------------------
