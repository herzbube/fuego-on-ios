//----------------------------------------------------------------------------
/** @file SgProcess.cpp
    See SgProcess.h */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgProcess.h"

#include "SgException.h"

// Not yet implemented for Windows or clang
#if defined(__GNUC__) && ! defined(__clang__)
#include <errno.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "SgDebug.h"
#include "SgStringUtil.h"

using std::vector;
using std::ios;

//----------------------------------------------------------------------------

namespace {

__gnu_cxx::stdio_filebuf<char>* CreateFileBuf(int fileDescriptor,
                                              std::ios_base::openmode mode)
{
#if __GNUC__ == 3 && __GNUC_MINOR__ >= 4
    return new __gnu_cxx::stdio_filebuf<char>(fileDescriptor, mode,
                                              static_cast<size_t>(BUFSIZ));
#elif __GNUC__ == 3
    return new __gnu_cxx::stdio_filebuf<char>(fileDescriptor, mode, true,
                                              static_cast<size_t>(BUFSIZ));
#else
    return new __gnu_cxx::stdio_filebuf<char>(fileDescriptor, mode);
#endif
}

void TerminateChild(const char* message)
{
    SgDebug() << message << '\n';
    exit(1);
}

} // namespace

//----------------------------------------------------------------------------

SgProcess::SgProcess(const std::string& command)
{
    vector<std::string> args = SgStringUtil::SplitArguments(command);
    if (args.size() == 0)
        throw SgException("Empty command line");
    int fd1[2];
    if (pipe(fd1) < 0)
        throw SgException("Pipe error");
    int fd2[2];
    if (pipe(fd2) < 0)
    {
        close(fd1[0]);
        close(fd1[1]);
        throw SgException("Pipe error");
    }
    pid_t pid;
    if ((pid = fork()) < 0)
        throw SgException("Fork error");
    else if (pid > 0) // Parent
    {
        close(fd1[0]);
        close(fd2[1]);
        m_bufOut.reset(CreateFileBuf(fd1[1], ios::out));
        m_out.reset(new std::ostream(m_bufOut.get()));
        m_bufIn.reset(CreateFileBuf(fd2[0], ios::in));
        m_in.reset(new std::istream(m_bufIn.get()));
        return;
    }
    else // Child
    {
        close(fd1[1]);
        close(fd2[0]);
        if (fd1[0] != STDIN_FILENO)
            if (dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO)
            {
                close(fd1[0]);
                TerminateChild("Error dup2 to stdin");
            }
        if (fd2[1] != STDOUT_FILENO)
            if (dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
            {
                close(fd2[1]);
                TerminateChild("Error dup2 to stdout");
            }
        char** const argv = new char*[args.size() + 1];
        for (size_t i = 0; i < args.size(); ++i)
        {
            argv[i] = new char[args[i].size()];
            strcpy(argv[i], args[i].c_str());
        }
        argv[args.size()] = 0;
        if (execvp(args[0].c_str(), argv) == -1)
            TerminateChild("Error execvp");
    }
}

#else

SgProcess::SgProcess(const std::string& command)
{
    SG_UNUSED(command);
    throw SgException("SgProcess not implemented on this architecture");
}

#endif // defined(__GNUC__) && ! defined(__clang__)

SgProcess::~SgProcess()
{ }

//----------------------------------------------------------------------------
