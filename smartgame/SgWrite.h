//----------------------------------------------------------------------------
/** @file SgWrite.h
    Utility write functions for SmartGo data types.
    @todo Write functions logically belong to the files of the classes they
    are outputting; should be moved. */
//----------------------------------------------------------------------------

#ifndef SG_WRITE_H
#define SG_WRITE_H

#include <iomanip>
#include <iosfwd>
#include <sstream>
#include <string>
#include <vector>
#include "SgArrayList.h"
#include "SgBoardColor.h"
#include "SgPoint.h"
#include "SgVector.h"

//----------------------------------------------------------------------------

/** Write left aligned text with a minimum width of 15 characters.
    Also appends a space to the label text. */
class SgWriteLabel
{
public:
    SgWriteLabel(const std::string& label)
        : m_label(label)
    { }

    friend std::ostream& operator<<(std::ostream& out,
                                    const SgWriteLabel& w);

private:
    std::string m_label;
};

//----------------------------------------------------------------------------

/** Write all points in list.
    Splits long lists into multiple lines. */
class SgWritePointList
{
public:
    SgWritePointList(const std::vector<SgPoint>& pointList,
                     std::string label = "",
                     bool writeSize = true);

    SgWritePointList(const SgVector<SgPoint>& pointList,
                     std::string label = "",
                     bool writeSize = true);

    std::ostream& Write(std::ostream& out) const;

private:
    bool m_writeSize;

    std::vector<SgPoint> m_pointList;

    std::string m_label;
};

/** @relatesalso SgWritePointList */
std::ostream& operator<<(std::ostream& out, const SgWritePointList& write);

//----------------------------------------------------------------------------

/** Output a SgArrayList with SgPoint elements to a stream. */
template<int SIZE>
class SgWriteSPointList
{
public:
    SgWriteSPointList(const SgArrayList<SgPoint, SIZE>& list,
                      std::string label = "", bool writeSize = true);

    std::ostream& Write(std::ostream& out) const;

private:
    bool m_writeSize;

    const SgArrayList<SgPoint,SIZE>& m_list;

    std::string m_label;
};

/** @relatesalso SgWriteSPointList */
template<int SIZE>
std::ostream& operator<<(std::ostream& out,
                         const SgWriteSPointList<SIZE>& write);

template<int SIZE>
SgWriteSPointList<SIZE>::SgWriteSPointList(
                                        const SgArrayList<SgPoint,SIZE>& list,
                                        std::string label, bool writeSize)
    : m_writeSize(writeSize),
      m_list(list),
      m_label(label)
{ }

template<int SIZE>
std::ostream& SgWriteSPointList<SIZE>::Write(std::ostream& out) const
{
    std::vector<SgPoint> list;
    for (typename SgArrayList<SgPoint,SIZE>::Iterator it(m_list); it; ++it)
        list.push_back(*it);
    return (out << SgWritePointList(list, m_label, m_writeSize));
}

template<int SIZE>
std::ostream& operator<<(std::ostream& out,
                         const SgWriteSPointList<SIZE>& write)
{
    return write.Write(out);
}

//----------------------------------------------------------------------------

/** Write line of dashes. */
class SgWriteLine
{
public:
    SgWriteLine()
    { }

    friend std::ostream& operator<<(std::ostream& out,
                                    const SgWriteLine &w);
};

//----------------------------------------------------------------------------

/** write "true" or "false"
    @todo just use boolalpha
*/
class SgWriteBoolean
{
public:
    explicit SgWriteBoolean(bool value) : m_value(value)
    { }

private:
    friend std::ostream& operator<<(std::ostream& out,
                                    const SgWriteBoolean &w);
    bool m_value;
};

//----------------------------------------------------------------------------

/** Writes boolean as 0/1. 
    @todo just use noboolalpha
*/
class SgWriteBoolAsInt
{
public:
    SgWriteBoolAsInt(bool value);

    std::ostream& Write(std::ostream& out) const;

private:
    bool m_value;
};

/** @relatesalso SgWriteBoolAsInt */
std::ostream& operator<<(std::ostream& out, const SgWriteBoolAsInt& write);

//----------------------------------------------------------------------------

#endif // SG_WRITE_H
