//----------------------------------------------------------------------------
/** @file SgSystem.h
    System specific definitions for SmartGo.

    This file contains system specific defines and includes.
    It always needs to be the first header file included by any .cpp file. */
//----------------------------------------------------------------------------

#ifndef SG_SYSTEM_H
#define SG_SYSTEM_H

//----------------------------------------------------------------------------
// Used by GNU Autotools
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//----------------------------------------------------------------------------

/** Avoid compiler warnings for unused variables.
    This function is more portable than using a \#pragma directive. */
template <class T>
inline void SG_UNUSED(const T&)
{ }

/** Avoid compiler warnings for variables used only if NDEBUG is not defined.
    This macro is more portable than using a \#pragma directive. */
#ifndef NDEBUG
#define SG_DEBUG_ONLY(x)
#else
#define SG_DEBUG_ONLY(x) SG_UNUSED(x)
#endif

//----------------------------------------------------------------------------

// Explicit inlining attributes. The macros are defined as non-empty only
// if supported by the compiler (note that Intel ICC and CLANG also define
// __GNUC__, but would ignore the attributes with a warning)

#if defined(__GNUC__) && ! defined(__ICC)
#define SG_ATTR_ALWAYS_INLINE __attribute__((always_inline))
#define SG_ATTR_NOINLINE __attribute__((noinline))
#else
#define SG_ATTR_NOINLINE
#define SG_ATTR_ALWAYS_INLINE
#endif

#if defined(__GNUC__) && ! defined(__ICC) && \
    ! defined(__clang__) && \
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#define SG_ATTR_FLATTEN __attribute__((flatten))
#else
#define SG_ATTR_FLATTEN
#endif

//----------------------------------------------------------------------------
/** Deterministic mode gives reproducible search results */
namespace SgDeterministic
{
   void SetDeterministicMode(bool flag);
   bool DeterministicMode();
}

/** Additional code to run in debug mode after an assertion failed. */
class SgAssertionHandler
{
public:
    /** Constructor.
        Automatically registers the handler. */
    SgAssertionHandler();

    /** Constructor.
        Automatically unregisters the handler. */
    virtual ~SgAssertionHandler();

    virtual void Run() = 0;
};

#ifndef NDEBUG

/** Help the Clang static analyzer use our assertions. 
    See http://clang-analyzer.llvm.org/annotations.html#attr_analyzer_noreturn
*/
#ifndef CLANG_ANALYZER_NORETURN
#if defined(__has_feature)
#if __has_feature(attribute_analyzer_noreturn)
#define CLANG_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
#endif
#endif
#endif
#ifndef CLANG_ANALYZER_NORETURN
#define CLANG_ANALYZER_NORETURN
#endif

/** System-specific action when an SG_ASSERT fails */
void SgHandleAssertion(const char* expr, const char* file, int line)
 CLANG_ANALYZER_NORETURN;

#define SG_ASSERT(x) \
    do \
    { \
        if (! (x)) \
            ::SgHandleAssertion(#x, __FILE__, __LINE__); \
    } while (false)
#else
#define SG_ASSERT(x) (static_cast<void>(0))
#endif

/** Convenience macro analogous to BOOST_CHECK_EQUAL */
#define SG_ASSERT_EQUAL(x, y) SG_ASSERT((x) == (y))

/** Check that i is within a given range.
    In many cases, doing two separate SG_ASSERT will be clearer 
    and give more information in case of failure.
*/
#define SG_ASSERTRANGE(i, from, to) SG_ASSERT((i) >= (from) && (i) <= (to))

//----------------------------------------------------------------------------

#ifndef NDEBUG
const bool SG_CHECK = true;
const bool SG_HEAVYCHECK = SG_CHECK && true;
#else
const bool SG_CHECK = false;
const bool SG_HEAVYCHECK = false;
#endif

//----------------------------------------------------------------------------

#ifdef _MSC_VER

// Don't report Visual C++ warning 4355 ('this' : used in base member
// initializer list) in default warning level 3. Storing a reference to
// 'this' is used at several places in the Fuego code (e.g. constructors of
// thread functions). This is not a problem as long as 'this' is not used
// yet.
#pragma warning(4:4355)

// Disable Visual C++ warnings about unsafe functions from the standard
// C++ library
#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#endif // _MSC_VER

//----------------------------------------------------------------------------

#ifdef __MINGW32__

#define WIN32 1

// Enable Windows2000 (0x0500) compatibility in MinGW header files
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

#endif // __MINGW32__

//----------------------------------------------------------------------------

/** Sets the global user abort flag.
    This flag should be set to false at the beginning of each user event,
    e.g. each GUI event or GTP command.
    Lengthy functions should poll the user abort flag with SgUserAbort and
    abort, if necessary; they should not reset the flag themselves.
    It can also be called from a different thread (the abort flag is
    declared volatile). */
void SgSetUserAbort(bool aborted);

/** Poll for user abort.
    @see SgSetUserAbort. */
bool SgUserAbort();

//----------------------------------------------------------------------------

inline void SgSynchronizeThreadMemory()
{
#ifdef ENABLE_CACHE_SYNC

#ifdef HAVE_SYNC_SYNCHRONIZE
    __sync_synchronize();
#else
#error "Explicit cache synchronization requires __sync_synchronize() builtin"
#endif

#endif
}

#endif // SG_SYSTEM_H
