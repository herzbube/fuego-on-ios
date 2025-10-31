//----------------------------------------------------------------------------
/** @file FuegoMainUtil.cpp
    See FuegoMainUtil.h */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "FuegoMainUtil.h"

#include <iostream>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/utility.hpp>
#include <fstream>
#include <sstream>
#include "FuegoMainEngine.h"
#include "FuegoMainUtil.h"
#include "GoBook.h"
#include "GoInit.h"
#include "SgDebug.h"
#include "SgException.h"
#include "SgInit.h"
#include "SgPlatform.h"
#include "SgStringUtil.h"

using boost::filesystem::path;
using std::ostream;
using std::string;
using std::vector;
namespace po = boost::program_options;

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

/** Get program directory from program path.
    @param programPath Program path taken from @c argv[0] in
    @c main. According to ANSI C, this can be @c 0. */
path GetProgramDir(const char* programPath)
{
    if (programPath == 0)
        return "";
    return path(programPath).parent_path();
}

path GetTopSourceDir()
{
    #ifdef ABS_TOP_SRCDIR
        return path(ABS_TOP_SRCDIR);
    #else
        return "";
    #endif
}

void Help(po::options_description& desc, ostream& out)
{
    out << "Usage: fuego [options] [input files]\n" << desc << "\n";
}

struct CommandLineOptions {

    /** Use opening book */
    bool m_useBook;

    /** Allow handicap games */
    bool m_allowHandicap;

    bool m_quiet;

    int m_fixedBoardSize;

    int m_maxGames;

    string m_config;

    const char* m_programPath;
    
    int m_srand;
    
    vector<string> m_inputFiles;
};

bool ParseOptions(int argc, char** argv, struct CommandLineOptions& options)
{
    po::options_description normalOptions("Options");
    normalOptions.add_options()
        ("config", 
         po::value<std::string>(&options.m_config)->default_value(""),
         "execute GTP commands from file before starting main command loop")
        ("help", "Displays this help and exit")
        ("maxgames", 
         po::value<int>(&options.m_maxGames)->default_value(-1),
         "make clear_board fail after n invocations")
        ("nobook", "don't automatically load opening book")
        ("nohandicap", "don't support handicap commands")
        ("quiet", "don't print debug messages")
        ("srand", 
         po::value<int>(&options.m_srand)->default_value(0),
         "set random seed (-1:none, 0:time(0))")
        ("size", 
         po::value<int>(&options.m_fixedBoardSize)->default_value(0),
         "initial (and fixed) board size");
    po::options_description hiddenOptions;
    hiddenOptions.add_options()
        ("input-file", po::value<vector<string> >(&options.m_inputFiles),
         "input file");
    po::options_description allOptions;
    allOptions.add(normalOptions).add(hiddenOptions);
    po::positional_options_description positionalOptions;
    positionalOptions.add("input-file", -1);
    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(allOptions).
                                     positional(positionalOptions).run(), vm);
        po::notify(vm);
    }
    catch (...)
    {
        Help(normalOptions, std::cerr);
    }
    if (vm.count("help"))
    {
        Help(normalOptions, std::cout);
        return false;
    }
    if (vm.count("nobook"))
        options.m_useBook = false;
    if (vm.count("nohandicap"))
        options.m_allowHandicap = false;
    if (vm.count("quiet"))
        options.m_quiet = true;
    return true;
}

void PrintStartupMessage()
{
    SgDebug() <<
        "Fuego " << FuegoMainUtil::Version() << "\n"
        "Copyright (C) 2009-2015 by the authors of the Fuego project.\n"
        "This program comes with ABSOLUTELY NO WARRANTY. This is\n"
        "free software and you are welcome to redistribute it under\n"
        "certain conditions. Type `fuego-license' for details.\n\n";
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

int FuegoMainUtil::FuegoMain(int argc, char** argv, std::istream* gtpClientInputStream, std::ostream* gtpClientOutputStream)
{
    /** Settings from command line options */
    struct CommandLineOptions options;
    options.m_useBook = true;
    options.m_allowHandicap = true;
    options.m_quiet = false;

    if (argc > 0 && argv != 0)
    {
        options.m_programPath = argv[0];
        SgPlatform::SetProgramDir(GetProgramDir(argv[0]));
        SgPlatform::SetTopSourceDir(GetTopSourceDir());
        try
        {
            bool okAndContinue = ParseOptions(argc, argv, options);
            if (! okAndContinue)
                return 0;
        }
        catch (const SgException& e)
        {
            SgDebug() << e.what() << "\n";
            return 1;
        }
    }
    if (options.m_quiet)
        SgDebugToNull();
    try
    {
        SgInit();
        GoInit();
        PrintStartupMessage();
        SgRandom::SetSeed(options.m_srand);
        FuegoMainEngine engine(options.m_fixedBoardSize,
                               options.m_programPath,
                               ! options.m_allowHandicap);
        GoGtpAssertionHandler assertionHandler(engine);
        if (options.m_maxGames >= 0)
            engine.SetMaxClearBoard(options.m_maxGames);
        if (options.m_useBook)
            FuegoMainUtil::LoadBook(engine.Book(), 
            					    SgPlatform::GetProgramDir());
        if (options.m_config != "")
            engine.ExecuteFile(options.m_config);

        std::istream* pInputStream;
        if (gtpClientInputStream)
            pInputStream = gtpClientInputStream;
        else
            pInputStream = &std::cin;

        std::ostream* pOutputStream;
        if (gtpClientOutputStream)
            pOutputStream = gtpClientOutputStream;
        else
            pOutputStream = &std::cout;

        if (! options.m_inputFiles.empty())
        {
            for (size_t i = 0; i < options.m_inputFiles.size(); i++)
            {
                string file = options.m_inputFiles[i];
                std::ifstream fin(file.c_str());
                if (! fin)
                    throw SgException(boost::format("Error file '%1%'") 
                    				  % file);
                pInputStream = &fin;
                GtpInputStream in(*pInputStream);
                GtpOutputStream out(*pOutputStream);
                engine.MainLoop(in, out);
            }
        }
        else
        {
            GtpInputStream in(*pInputStream);
            GtpOutputStream out(*pOutputStream);
            engine.MainLoop(in, out);
        }
    }
    catch (const GtpFailure& e)
    {
        SgDebug() << e.Response() << '\n';
        return 1;
    }
    catch (const std::exception& e)
    {
        SgDebug() << e.what() << '\n';
        return 1;
    }
    return 0;
}

//----------------------------------------------------------------------------
