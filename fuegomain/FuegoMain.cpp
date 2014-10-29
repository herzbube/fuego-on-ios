//----------------------------------------------------------------------------
/** @file FuegoMain.cpp
    Main function for Fuego */
//----------------------------------------------------------------------------

#include "SgSystem.h"

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
#include "FuegoMainEngine.h"
#include "FuegoMainUtil.h"
#include "GoInit.h"
#include "SgDebug.h"
#include "SgException.h"
#include "SgInit.h"

using namespace std;
using boost::filesystem::path;
using boost::format;
namespace po = boost::program_options;

//----------------------------------------------------------------------------

namespace {

/** @name Settings from command line options */
// @{

bool g_noHandicap = false;

bool g_noBook = false;

bool g_quiet = false;

int g_fixedBoardSize;

int g_maxGames;

string g_config;

path g_programDir;

const char* g_programPath;

int g_srand;

vector<string> g_inputFiles;

// @} // @name

/** Get program directory from program path.
    @param programPath Program path taken from @c argv[0] in
    @c main. According to ANSI C, this can be @c 0. */
path GetProgramDir(const char* programPath)
{
    if (programPath == 0)
        return "";
    # if defined(BOOST_FILESYSTEM_VERSION)
        SG_ASSERT (  BOOST_FILESYSTEM_VERSION == 2
                  || BOOST_FILESYSTEM_VERSION == 3);
    #endif

    #if (defined(BOOST_FILESYSTEM_VERSION) && (BOOST_FILESYSTEM_VERSION == 3))
        return path(programPath).branch_path();
    #else
        return path(programPath, boost::filesystem::native).branch_path();
    #endif	
}

void Help(po::options_description& desc, ostream& out)
{
    out << "Usage: fuego [options] [input files]\n" << desc << "\n";
    exit(0);
}

void ParseOptions(int argc, char** argv)
{
    po::options_description normalOptions("Options");
    normalOptions.add_options()
        ("config", 
         po::value<std::string>(&g_config)->default_value(""),
         "execuate GTP commands from file before starting main command loop")
        ("help", "Displays this help and exit")
        ("maxgames", 
         po::value<int>(&g_maxGames)->default_value(-1),
         "make clear_board fail after n invocations")
        ("nobook", "don't automatically load opening book")
        ("nohandicap", "don't support handicap commands")
        ("quiet", "don't print debug messages")
        ("srand", 
         po::value<int>(&g_srand)->default_value(0),
         "set random seed (-1:none, 0:time(0))")
        ("size", 
         po::value<int>(&g_fixedBoardSize)->default_value(0),
         "initial (and fixed) board size");
    po::options_description hiddenOptions;
    hiddenOptions.add_options()
        ("input-file", po::value<vector<string> >(&g_inputFiles),
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
        Help(normalOptions, cerr);
    }
    if (vm.count("help"))
        Help(normalOptions, cout);
    if (vm.count("nobook"))
        g_noBook = true;
    if (vm.count("nohandicap"))
        g_noHandicap = true;
    if (vm.count("quiet"))
        g_quiet = true;
}

void PrintStartupMessage()
{
    SgDebug() <<
        "Fuego " << FuegoMainUtil::Version() << "\n"
        "Copyright (C) 2009-2011 by the authors of the Fuego project.\n"
        "This program comes with ABSOLUTELY NO WARRANTY. This is\n"
        "free software and you are welcome to redistribute it under\n"
        "certain conditions. Type `fuego-license' for details.\n\n";
}

} // namespace

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc > 0 && argv != 0)
    {
        g_programPath = argv[0];
        g_programDir = GetProgramDir(argv[0]);
        try
        {
            ParseOptions(argc, argv);
        }
        catch (const SgException& e)
        {
            SgDebug() << e.what() << "\n";
            return 1;
        }
    }
    if (g_quiet)
        SgDebugToNull();
    try
    {
        SgInit();
        GoInit();
        PrintStartupMessage();
        SgRandom::SetSeed(g_srand);
        FuegoMainEngine engine(g_fixedBoardSize, g_programPath, g_noHandicap);
        GoGtpAssertionHandler assertionHandler(engine);
        if (g_maxGames >= 0)
            engine.SetMaxClearBoard(g_maxGames);
        if (! g_noBook)
            FuegoMainUtil::LoadBook(engine.Book(), g_programDir);
        if (g_config != "")
            engine.ExecuteFile(g_config);
        if (! g_inputFiles.empty())
        {
            for(size_t i = 0; i < g_inputFiles.size(); i++)
            {
                string file = g_inputFiles[i];
                ifstream fin(file.c_str());
                if (! fin)
                    throw SgException(format("Error file '%1%'") % file);
                GtpInputStream in(fin);
                GtpOutputStream out(cout);
                engine.MainLoop(in, out);
            }
        }
        else
        {
            GtpInputStream in(cin);
            GtpOutputStream out(cout);
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

