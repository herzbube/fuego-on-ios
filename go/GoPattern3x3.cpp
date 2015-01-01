//----------------------------------------------------------------------------
/** @file GoPattern3x3.cpp
    See GoPattern3x3.h */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "GoPatternBase.h"
#include "GoPattern3x3.h"

#include "GoBoardUtil.h"
#include "SgDebug.h"

//----------------------------------------------------------------------------

namespace {
    
bool CheckCut1(const GoBoard& bd, SgPoint p,
               SgBlackWhite c, int cDir, int otherDir)
{
    SG_ASSERT_BW(c);
    return bd.IsColor(p + otherDir, c)
        && bd.IsColor(p + cDir + otherDir, SgOppBW(c));
}

bool CheckCut2(const GoBoard& bd, SgPoint p,
               const SgBlackWhite c, int cDir,
               int otherDir)
{
    SG_ASSERT_BW(c);
    SG_ASSERT(bd.IsColor(p + cDir, c));
    const SgBlackWhite opp = SgOppBW(c);
    return bd.IsColor(p - cDir, c)
        && ( (  bd.IsColor(p + otherDir, opp)
              && ! bd.IsColor(p - otherDir + cDir, c)
              && ! bd.IsColor(p - otherDir - cDir, c)
              )
            ||
            (  bd.IsColor(p - otherDir, opp)
             && ! bd.IsColor(p + otherDir + cDir, c)
             && ! bd.IsColor(p + otherDir - cDir, c)
             )
            );
}

bool CheckHane1(const GoBoard& bd, SgPoint p,
                SgBlackWhite c, SgBlackWhite opp,
                int cDir, int otherDir)
{
    return bd.IsColor(p + cDir, c)
        && bd.IsColor(p + cDir + otherDir, opp)
        && bd.IsColor(p + cDir - otherDir, opp)
        && bd.IsEmpty(p + otherDir)
        && bd.IsEmpty(p - otherDir)
        ;
}

int EdgeDirection(GoBoard& bd, SgPoint p, int index)
{
    const int up = bd.Up(p);
    switch (index)
    {
        case 0:
        return GoPatternBase::OtherDir(up);
        case 1:
        return up + GoPatternBase::OtherDir(up);
        case 2:
        return up;
        case 3:
        return up - GoPatternBase::OtherDir(up);
        default:
        SG_ASSERT(index == 4);
        return - GoPatternBase::OtherDir(up);
    }
}

/** Find direction of a neighboring stone in color c */
int FindDir(const GoBoard& bd, SgPoint p, SgBlackWhite c)
{
    if (bd.IsColor(p + SG_NS, c))
        return SG_NS;
    if (bd.IsColor(p - SG_NS, c))
        return -SG_NS;
    if (bd.IsColor(p + SG_WE, c))
        return SG_WE;
    SG_ASSERT(bd.IsColor(p - SG_WE, c));
    return -SG_WE;
}

bool MatchCut(const GoBoard& bd, SgPoint p)
{
    if (bd.Num8EmptyNeighbors(p) > 6)
        return false;
    
    const int nuEmpty = bd.NumEmptyNeighbors(p);
    //cut1
    const SgEmptyBlackWhite c1 = bd.GetColor(p + SG_NS);
    if (   c1 != SG_EMPTY
        && bd.NumNeighbors(p, c1) >= 2
        && ! (bd.NumNeighbors(p, c1) == 3 && nuEmpty == 1)
        && (  CheckCut1(bd, p, c1, SG_NS, SG_WE)
            || CheckCut1(bd, p, c1, SG_NS, -SG_WE)
            )
        )
        return true;
    const SgEmptyBlackWhite c2 = bd.GetColor(p - SG_NS);
    if (  c2 != SG_EMPTY
        && bd.NumNeighbors(p, c2) >= 2
        && ! (bd.NumNeighbors(p, c2) == 3 && nuEmpty == 1)
        && (  CheckCut1(bd, p, c2, -SG_NS, SG_WE)
            || CheckCut1(bd, p, c2, -SG_NS, -SG_WE)
            )
        )
        return true;
    //cut2
    if (  c1 != SG_EMPTY
        && bd.NumNeighbors(p, c1) == 2
        && bd.NumNeighbors(p, SgOppBW(c1)) > 0
        && bd.NumDiagonals(p, c1) <= 2
        && CheckCut2(bd, p, c1, SG_NS, SG_WE)
        )
        return true;
    const SgEmptyBlackWhite c3 = bd.GetColor(p + SG_WE);
    if (  c3 != SG_EMPTY
        && bd.NumNeighbors(p, c3) == 2
        && bd.NumNeighbors(p, SgOppBW(c3)) > 0
        && bd.NumDiagonals(p, c3) <= 2
        && CheckCut2(bd, p, c3, SG_WE, SG_NS)
        )
        return true;
    return false;
}

bool MatchEdge(const GoBoard& bd, SgPoint p,
               const int nuBlack, const int nuWhite)
{
    const int up = bd.Up(p);
    const int side = GoPatternBase::OtherDir(up);
    const int nuEmpty = bd.NumEmptyNeighbors(p);
    const SgEmptyBlackWhite upColor = bd.GetColor(p + up);
    // edge1
    if (  nuEmpty > 0
        && (nuBlack > 0 || nuWhite > 0)
        && upColor == SG_EMPTY
        )
    {
        const SgEmptyBlackWhite c1 = bd.GetColor(p + side);
        if (c1 != SG_EMPTY && bd.GetColor(p + side + up) == SgOppBW(c1))
            return true;
        const SgEmptyBlackWhite c2 = bd.GetColor(p - side);
        if (c2 != SG_EMPTY && bd.GetColor(p - side + up) == SgOppBW(c2))
            return true;
    }
    
    // edge2
    if (  upColor != SG_EMPTY
        && (  (upColor == SG_BLACK && nuBlack == 1 && nuWhite > 0)
            || (upColor == SG_WHITE && nuWhite == 1 && nuBlack > 0)
            )
        )
        return true;
    
    const SgBlackWhite toPlay = bd.ToPlay();
    // edge3
    if (  upColor == toPlay
        && bd.NumDiagonals(p, SgOppBW(upColor)) > 0
        )
        return true;
    
    // edge4
    if (  upColor == SgOppBW(toPlay)
        && bd.NumNeighbors(p, upColor) <= 2
        && bd.NumDiagonals(p, toPlay) > 0
        )
    {
        if (  bd.GetColor(p + side + up) == toPlay
            && bd.GetColor(p + side) != upColor
            )
            return true;
        if (  bd.GetColor(p - side + up) == toPlay
            && bd.GetColor(p - side) != upColor
            )
            return true;
    }
    // edge5
    if (  upColor == SgOppBW(toPlay)
        && bd.NumNeighbors(p, upColor) == 2
        && bd.NumNeighbors(p, toPlay) == 1
        )
    {
        if (  bd.GetColor(p + side + up) == toPlay
            && bd.GetColor(p + side) == upColor
            )
            return true;
        if (  bd.GetColor(p - side + up) == toPlay
            && bd.GetColor(p - side) == upColor
            )
            return true;
    }
    return false;
}

bool MatchHane(const GoBoard& bd, SgPoint p,
               const int nuBlack, const int nuWhite)
{
    const int nuEmpty = bd.NumEmptyNeighbors(p);
    if (nuEmpty < 2 || nuEmpty > 3)
        return false;
    if (  (nuBlack < 1 || nuBlack > 2)
        && (nuWhite < 1 || nuWhite > 2)
        )
        return false;
    if (nuEmpty == 2) // hane3 pattern
    {
        if (nuBlack == 1 && nuWhite == 1)
        {
            const int dirB = FindDir(bd, p, SG_BLACK);
            const int dirW = FindDir(bd, p, SG_WHITE);
            if (! bd.IsEmpty(p + dirB + dirW))
                return true;
        }
    }
    else if (nuEmpty == 3) // hane2 or hane4
    {
        SG_ASSERT(nuBlack + nuWhite == 1);
        const SgBlackWhite col = (nuBlack == 1) ? SG_BLACK : SG_WHITE;
        const SgBlackWhite opp = SgOppBW(col);
        const int dir = FindDir(bd, p, col);
        const int otherDir = GoPatternBase::OtherDir(dir);
        if (  bd.IsEmpty(p + dir + otherDir)
            && bd.IsColor(p + dir - otherDir, opp)
            )
            return true; // hane2
        if (  bd.IsEmpty(p + dir - otherDir)
            && bd.IsColor(p + dir + otherDir, opp)
            )
            return true; // hane2
        if (bd.ToPlay() == opp)
        {
            const SgEmptyBlackWhite c1 = bd.GetColor(p + dir + otherDir);
            if (c1 != SG_EMPTY)
            {
                const SgEmptyBlackWhite c2 =
                bd.GetColor(p + dir - otherDir);
                if (SgOppBW(c1) == c2)
                    return true; // hane4
            }
        }
    }
    
    // hane1 pattern
    const int nuBlackDiag = bd.NumDiagonals(p, SG_BLACK);
    if (  nuBlackDiag >= 2
        && nuWhite > 0
        && (  CheckHane1(bd, p, SG_WHITE, SG_BLACK, SG_NS, SG_WE)
            || CheckHane1(bd, p, SG_WHITE, SG_BLACK, -SG_NS, SG_WE)
            || CheckHane1(bd, p, SG_WHITE, SG_BLACK, SG_WE, SG_NS)
            || CheckHane1(bd, p, SG_WHITE, SG_BLACK, -SG_WE, SG_NS)
            )
        )
        return true;
    
    const int nuWhiteDiag = bd.NumDiagonals(p, SG_WHITE);
    if (  nuWhiteDiag >= 2
        && nuBlack > 0
        && (  CheckHane1(bd, p, SG_BLACK, SG_WHITE, SG_NS, SG_WE)
            || CheckHane1(bd, p, SG_BLACK, SG_WHITE, -SG_NS, SG_WE)
            || CheckHane1(bd, p, SG_BLACK, SG_WHITE, SG_WE, SG_NS)
            || CheckHane1(bd, p, SG_BLACK, SG_WHITE, -SG_WE, SG_NS)
            )
        )
        return true;
    return false;
}

/* Slow, used only to initialize static table */
int SwapColor(int origCode, size_t length)
{
    const std::vector<SgEmptyBlackWhite> colors =
    GoPatternBase::Decode(origCode, length);

    int code = 0;
    for (std::vector<SgEmptyBlackWhite>::const_iterator it = colors.begin();
         it != colors.end(); ++it)
    {
        code *= 3;
        code += SgOpp(*it);
    }
    return code;
}

template<std::size_t SIZE>
void ComputeSwappedTable(const boost::array<int, SIZE>& blackTable,
                         boost::array<int, SIZE>& whiteTable,
                         const size_t length)
{
    SG_ASSERT(length == 5 || length == 8);
    if (length == 5)
        SG_ASSERT(SIZE == GoPattern3x3::GOUCT_POWER3_5);
    if (length == 8)
        SG_ASSERT(SIZE == GoPattern3x3::GOUCT_POWER3_8);

    for (int i = 0; i < static_cast<int>(SIZE); ++i)
    {
        const int j = SwapColor(i, length);
        whiteTable[i] = blackTable[j];
        SG_ASSERT(SwapColor(j, length) == i);
    }
}


void SetupCodedEdgePosition(GoBoard& bd, int code)
{
    const int origCode = code;
    SG_DEBUG_ONLY(origCode);
    
    const SgPoint p = SgPointUtil::Pt(1, 3);
    for (int i = 4; i >= 0; --i) // decoding gives points in reverse order
    {
        const SgPoint nb = p + EdgeDirection(bd, p, i);
        int c = code % 3;
        code /= 3;
        if (c != SG_EMPTY)
        {
            SG_ASSERT(bd.IsLegal(nb, c));
            bd.Play(nb, c);
        }
    }
    SG_ASSERT(GoPattern3x3::CodeOfEdgeNeighbors(bd, p) == origCode);
}

void SetupCodedPosition(GoBoard& bd, int code)
{
    const int origCode = code;
    SG_DEBUG_ONLY(origCode);
    
    const SgPoint p = SgPointUtil::Pt(3, 3);
    for (int i = 7; i >= 0; --i) // decoding gives points in reverse order
    {
        const SgPoint nb = p + SgNb8Iterator::Direction(i);
        int c = code % 3;
        code /= 3;
        if (c != SG_EMPTY)
        {
            SG_ASSERT(bd.IsLegal(nb, c));
            bd.Play(nb, c);
        }
    }
    SG_ASSERT(GoPattern3x3::CodeOf8Neighbors(bd, p) == origCode);
}

const int PA_NU_AXES = 3;

const unsigned int PA_NU_AX_SETS = 1 << PA_NU_AXES;

typedef unsigned int PaAxSet;
// todo copied from features


int AxCodeOf8Neighbors(const GoBoard& bd, SgPoint p, PaAxSet axes)
{
    using std::make_pair;
    using GoPatternBase::EBWCodeOfPoint;

    static const std::pair<int,int> axDir[PA_NU_AX_SETS] =
    {
        make_pair( SG_NS,  SG_WE),
        make_pair( SG_NS, -SG_WE),
        make_pair(-SG_NS,  SG_WE),
        make_pair(-SG_NS, -SG_WE),
        make_pair( SG_WE,  SG_NS),
        make_pair( SG_WE, -SG_NS),
        make_pair(-SG_WE,  SG_NS),
        make_pair(-SG_WE, -SG_NS)
    };
    
    const std::pair<int,int> dir = axDir[axes];
    SG_ASSERT(bd.Line(p) > 1);
    int code = ((((((   EBWCodeOfPoint(bd, p + dir.first + dir.second) * 3
                     + EBWCodeOfPoint(bd, p + dir.first)) * 3
                    + EBWCodeOfPoint(bd, p + dir.first - dir.second)) * 3
                   + EBWCodeOfPoint(bd, p + dir.second)) * 3
                  + EBWCodeOfPoint(bd, p - dir.second)) * 3
                 + EBWCodeOfPoint(bd, p - dir.first + dir.second)) * 3
                + EBWCodeOfPoint(bd, p - dir.first)) * 3
    + EBWCodeOfPoint(bd, p - dir.first - dir.second);
    SG_ASSERT(code >= 0);
    SG_ASSERT(code < GoPattern3x3::GOUCT_POWER3_8);
    return code;
}

int MinCodeOf8Neighbors(const GoBoard& bd, SgPoint p)
{
    int minCode = GoPattern3x3::GOUCT_POWER3_8;
    for (PaAxSet s = 0; s < PA_NU_AX_SETS; ++s)
    {
        const int code = AxCodeOf8Neighbors(bd, p, s);
        minCode = std::min(minCode, code);
    }
    SG_ASSERT(minCode >= 0);
    SG_ASSERT(minCode < GoPattern3x3::GOUCT_POWER3_8);
    return minCode;
}

int MirrorCodeOfEdgeNeighbors(const GoBoard& bd,
                              SgPoint p)
{
    using GoPatternBase::EBWCodeOfPoint;
    SG_ASSERT(bd.Line(p) == 1);
    SG_ASSERT(bd.Pos(p) > 1);
    const int up = bd.Up(p);
    int other = -GoPatternBase::OtherDir(up);
    int code = (((EBWCodeOfPoint(bd, p + other) * 3
                  + EBWCodeOfPoint(bd, p + up + other)) * 3
                 + EBWCodeOfPoint(bd, p + up)) * 3
                + EBWCodeOfPoint(bd, p + up - other)) * 3
    + EBWCodeOfPoint(bd, p - other);
    SG_ASSERT(code >= 0);
    SG_ASSERT(code < GoPattern3x3::GOUCT_POWER3_5);
    return code;
}

int MinEdgeCode(const GoBoard& bd, SgPoint p)
{
    return std::min(GoPattern3x3::CodeOfEdgeNeighbors(bd, p),
                    MirrorCodeOfEdgeNeighbors(bd, p));
}
    
void PrintVector(const std::vector<int>& codes,
                 SgBWArray<GoPattern3x3::GoUctEdgePatternTable>& edgeTable)
{
    SgDebug() << "Shared: ";
    
    for (std::vector<int>::const_iterator it = codes.begin();
         it != codes.end(); ++it)
    {
        SgDebug() << "Code " << *it <<
        " gamma B = " << edgeTable[SG_BLACK][*it].GetGammaValue()
        << " gamma W = " << edgeTable[SG_WHITE][*it].GetGammaValue()
        ;
    }
    SgDebug() << '\n';
}

void PrintVector(const std::vector<int>& codes,
                 SgBWArray<GoPattern3x3::GoUctPatternTable>& table)
{
    SgDebug() << "Shared: ";
    
    for (std::vector<int>::const_iterator it = codes.begin();
         it != codes.end(); ++it)
    {
        SgDebug() << "Code " << *it <<
        " gamma B = " << table[SG_BLACK][*it].GetGammaValue()
        << " gamma W = " << table[SG_WHITE][*it].GetGammaValue();
    }
    SgDebug() << '\n';
}

    
} // namespace

//----------------------------------------------------------------------------

int GoPattern3x3::SwapCenterColor(int code)
{
    return SwapColor(code, 8);
}

int GoPattern3x3::SwapEdgeColor(int code)
{
    return SwapColor(code, 5);
}

int GoPattern3x3::Map2x3EdgeCode(int code, SgBlackWhite toPlay)
{
    static EdgeCodeTable s_indexCode[2];
    static bool s_initialized = false;
    BOOST_STATIC_ASSERT(SG_BLACK == 0);
    BOOST_STATIC_ASSERT(SG_WHITE == 1);
    
    if (! s_initialized)
    {
        MapEdgePatternsToMinimum(s_indexCode[0]);
        ComputeSwappedTable(s_indexCode[0], s_indexCode[1], 5);
        s_initialized = true;
    }
    
    SG_ASSERT(code >= 0);
    SG_ASSERT(code < GOUCT_POWER3_5);
    return s_indexCode[toPlay][code];
}

int GoPattern3x3::Map3x3CenterCode(int code, SgBlackWhite toPlay)
{
    //-------------------------------
    static CenterCodeTable s_indexCode[2];
    static bool s_initialized = false;
    
    if (! s_initialized)
    {
        MapCenterPatternsToMinimum(s_indexCode[0]);
        ComputeSwappedTable(s_indexCode[0], s_indexCode[1], 8);
        s_initialized = true;
    }
    //-------------------------------

    SG_ASSERT(code >= 0);
    SG_ASSERT(code < GOUCT_POWER3_8);
    return s_indexCode[toPlay][code];
}

int GoPattern3x3::DecodeEdgeIndex(int index)
{
    SG_ASSERT(index >= 0);
    SG_ASSERT(index < GOUCT_POWER3_5); // todo get, store real max. index
    for (int i = 0; i < GOUCT_POWER3_5; ++i)
        if (Map2x3EdgeCode(i, SG_BLACK) == index) // TODO review
            return i;
    SG_ASSERT(false);
    return -1;
}

int GoPattern3x3::DecodeCenterIndex(int index)
// todo can speed up by computing reverse map if needed
{
    SG_ASSERT(index >= 0);
    SG_ASSERT(index < GOUCT_POWER3_8); // todo get, store real max. index
    for (int i = 0; i < GOUCT_POWER3_8; ++i)
        if (Map3x3CenterCode(i, SG_BLACK) == index)  // TODO review
            return i;
    SG_ASSERT(false);
    return -1;
}

int GoPattern3x3::MapCenterPatternsToMinimum(CenterCodeTable& indexCode)
{
    GoBoard bd(5);
    CenterCodeTable minCode;
    const SgPoint p = SgPointUtil::Pt(3, 3);
    for (int i = 0; i < GOUCT_POWER3_8; ++i)
    {
        SetupCodedPosition(bd, i);
        minCode[i] = MinCodeOf8Neighbors(bd, p);
        GoBoardUtil::UndoAll(bd);
    }
    CenterCodeTable uniqueCode;
    std::copy(minCode.begin(), minCode.end(), uniqueCode.begin());
    std::sort(uniqueCode.begin(), uniqueCode.end());
    int* last = std::unique(uniqueCode.begin(), uniqueCode.end());
    //SgDebug() <<  last - uniqueCode << " unique elements " << '\n';
    for (int i = 0; i < GOUCT_POWER3_8; ++i)
    {
        int* index = std::lower_bound(uniqueCode.begin(), last, minCode[i]);
        SG_ASSERT(index >= uniqueCode.begin());
        SG_ASSERT(index < last);
        SG_ASSERT(index + 1 == std::upper_bound(uniqueCode.begin(),
                                                last, minCode[i]));
        indexCode[i] = static_cast<int>(index - uniqueCode.begin());
        //Write3x3CenterPattern(SgDebug(), i);
        //SgDebug() << "indexCode[" <<  i << "] = " << indexCode[i] << '\n';
    }
    return static_cast<int>(last - uniqueCode.begin());
}

int GoPattern3x3::MapEdgePatternsToMinimum(EdgeCodeTable& indexCode)
{
    GoBoard bd(5);
    EdgeCodeTable minCode;
    const SgPoint p = SgPointUtil::Pt(1, 3);
    for (int i = 0; i < GOUCT_POWER3_5; ++i)
    {
        SetupCodedEdgePosition(bd, i);
        minCode[i] = MinEdgeCode(bd, p);
        GoBoardUtil::UndoAll(bd);
    }
    EdgeCodeTable uniqueCode;
    std::copy(minCode.begin(), minCode.end(), uniqueCode.begin());
    std::sort(uniqueCode.begin(), uniqueCode.end());
    int* last = std::unique(uniqueCode.begin(), uniqueCode.end());
    //SgDebug() << '\n' <<  last - uniqueCode.begin() << " unique elements " << '\n';
    for (int i = 0; i < GOUCT_POWER3_5; ++i)
    {
        int* index = std::lower_bound(uniqueCode.begin(), last, minCode[i]);
        SG_ASSERT(index >= uniqueCode.begin());
        SG_ASSERT(index < last);
        SG_ASSERT(index + 1 == std::upper_bound(uniqueCode.begin(),
                                                last, minCode[i]));
        indexCode[i] = static_cast<int>(index - uniqueCode.begin());
        //Write2x3EdgePattern(SgDebug(), i);
        //SgDebug() << "indexCode[" <<  i << "] = " << indexCode[SG_BLACK][i] << '\n';
    }
    return static_cast<int>(last - uniqueCode.begin());
}

void GoPattern3x3::InitEdgePatternTable(SgBWArray<GoUctEdgePatternTable>&
                                      edgeTable)
{
    GoBoard bd(5);
    const SgPoint p = SgPointUtil::Pt(1, 3);
    for (int i = 0; i < GOUCT_POWER3_5; ++i)
    {
        SetupCodedEdgePosition(bd, i);
        for (SgBWIterator it; it; ++it)
        {
            bd.SetToPlay(*it);
            const bool isPattern = GoPattern3x3::MatchAnyPattern(bd, p);
            edgeTable[*it][i].SetIsPattern(isPattern);
        }
        GoBoardUtil::UndoAll(bd);
    }
    //ReduceEdgeSymmetry(edgeTable);
}

void GoPattern3x3::InitCenterPatternTable(SgBWArray<GoUctPatternTable>& table)
{
    GoBoard bd(5);
    const SgPoint p = SgPointUtil::Pt(3, 3);
    for (int i = 0; i < GOUCT_POWER3_8; ++i)
    {
        SetupCodedPosition(bd, i);
        for (SgBWIterator it; it; ++it)
        {
            bd.SetToPlay(*it);
            const bool isPattern = GoPattern3x3::MatchAnyPattern(bd, p);
            table[*it][i].SetIsPattern(isPattern);
        }
        GoBoardUtil::UndoAll(bd);
    }
    //ReduceCenterSymmetry(table);
}


#if 0

GetCodesForFeature(int i)
{
    
}

// copy symmetry-reduced features read from a file
// into a full table
StoreFeatureValues()
{
    for (int i = 0; i < nuFeatures; ++i)
    {
        codes = GetCodesForFeature(i);
        for (vector<int>::const_iterator it = codes.begin();
             it != codes.end(); ++it)
        table[code].SetGamma(featureValue[i]);
    }
}
#endif

namespace GoPattern3x3 {

class TwoWay3x3Map
{
public:
    
    TwoWay3x3Map();

    int m_nuUnique;

    CenterCodeTable m_indexCode;
    
    CenterCodeTable m_centerSwapTable;
    
    std::vector<std::vector<int> > m_uniqueCode;

    // length m_nuUnique

private:
    void Init();
};

} // namespace GoPattern3x3

GoPattern3x3::TwoWay3x3Map::TwoWay3x3Map()
    : m_nuUnique(0)
{
    Init();
}

void GoPattern3x3::TwoWay3x3Map::Init()
{
    m_nuUnique = MapCenterPatternsToMinimum(m_indexCode);
    m_uniqueCode.resize(m_nuUnique);
    for (int i = 0; i < GOUCT_POWER3_8; ++i)
    {
        int u = m_indexCode[i];
        SG_ASSERT(u < m_nuUnique);
        m_uniqueCode[u].push_back(i);
    }
    for (int i = 0; i < GOUCT_POWER3_8; ++i)
    m_centerSwapTable[i] = SwapCenterColor(i);
    // swap twice gives original TODO move to unit test.
    for (int i = 0; i < GOUCT_POWER3_8; ++i)
    SG_ASSERT_EQUAL(m_centerSwapTable[m_centerSwapTable[i]], i);
}

void GoPattern3x3::ReduceCenterSymmetry(SgBWArray<GoUctPatternTable>& table)
{
    TwoWay3x3Map m;
    for (int i = 0; i < m.m_nuUnique; ++i)
    PrintVector(m.m_uniqueCode[i], table); // TODO white
}

void GoPattern3x3::ReduceEdgeSymmetry(SgBWArray<GoUctEdgePatternTable>&
                                    edgeTable)
{
    boost::array<int, GOUCT_POWER3_5> indexCode;
    int nuUnique = MapEdgePatternsToMinimum(indexCode);
    std::vector<int> uCode[GOUCT_POWER3_5]; // nuUnique
    for (int i = 0; i < GOUCT_POWER3_5; ++i)
    {
        int u = indexCode[i];
        SG_ASSERT(u < nuUnique);
        uCode[u].push_back(i);
    }
    for (int i = 0; i < nuUnique; ++i)
        PrintVector(uCode[i], edgeTable);
}

bool GoPattern3x3::MatchAnyPattern(const GoBoard& bd, SgPoint p)
{
    SG_ASSERT(bd.IsEmpty(p));
    const int nuBlack = bd.NumNeighbors(p, SG_BLACK);
    const int nuWhite = bd.NumNeighbors(p, SG_WHITE);
    
    // Quick refutation using the incremental neighbor counts of the board
    // All patterns have at least one adjacent stone
    if (nuBlack == 0 && nuWhite == 0)
        return false;
    
    // Filter edge moves on (1,1) points in corners
    if (bd.Pos(p) == 1)
        return false;
    if (bd.Line(p) == 1)
        return MatchEdge(bd, p, nuBlack, nuWhite);
    else // Center
        return MatchHane(bd, p, nuBlack, nuWhite)
            || MatchCut(bd, p);
}

void GoPattern3x3::Write2x3EdgePattern(std::ostream& stream, int code)
{
    stream << '\n';
    GoBoard bd(5);
    SetupCodedEdgePosition(bd, code);
    for (int i = 1; i <= 2; ++i)
    {
        for (int j = 2; j <= 4; ++j)
        {
            const SgPoint p = SgPointUtil::Pt(i, j);
            stream << SgEBW(bd.GetColor(p));
        }
        stream << '\n';
    }
}

void GoPattern3x3::Write3x3CenterPattern(std::ostream& stream, int code)
{
    stream << '\n';
    GoBoard bd(5);
    SetupCodedPosition(bd, code);
    for (int i = 4; i >= 2; --i)
    {
        for (int j = 2; j <= 4; ++j)
        {
            const SgPoint p = SgPointUtil::Pt(j, i);
            stream << SgEBW(bd.GetColor(p));
        }
        stream << '\n';
    }
}

//----------------------------------------------------------------------------
