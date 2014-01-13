//----------------------------------------------------------------------------
/** @file GoEyeUtil.h
    GoBoard eye-related utility classes. */
//----------------------------------------------------------------------------

#ifndef GO_EYEUTIL_H
#define GO_EYEUTIL_H

#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "SgPoint.h"

//----------------------------------------------------------------------------

namespace GoEyeUtil
{
    /** size of largest standard nakade shape */
    const int NAKADE_LIMIT = 6;

    /** Given opponent's safe stones, can p ever become an eye?
     Checks direct and diagonal neighbors. */
    bool CanBecomeSinglePointEye(const GoBoard& bd, SgPoint p,
                                 const SgPointSet& oppSafe);
    
    /** does playing at move make p into a single point eye? */
    template<class BOARD>
    bool CanMakeEye(const BOARD& bd, SgBlackWhite color,
                    SgPoint p, SgPoint& move);

    /** Code for how many points of each degree there are.
        The degree measures how many of the (up to 4) neighbors
        are also in the set of points.

        code =     1 * # degree 0
             +    10 * # degree 1
             +   100 * # degree 2
             +  1000 * # degree 3
             + 10000 * # degree 4

        This is a different format, but has the same information
        as the Cazenave/Vila "neighbour classification".
        E.g. their code 112224 means 2x degree 1, 3x degree 2, 1x degree 4,
        so the DegreeCode is 10320.

        The DegreeCode is not strong enough for graph isomorphism testing -
        there are nonisomorphic graphs with the same code -
        but it is good for distinguishing small graphs.

        For example, it can not distinguish between a "straight" and a "bent"
        line of three. */
    int DegreeCode(const SgPointSet& points);

    /** Like DegreeCode, but also count diagonal neighbors */
    long DegreeCode8(const SgPointSet& points);

    /** Return an empty neighbor of p. Precondition: one must exist. */
    template<class BOARD>
    SgPoint EmptyNeighbor(const BOARD& bd, SgPoint p);

    /** Check if area is one of the classical nakade shapes:
        *,**,***, **, ***, **, ***,  * ,  *  .
                   *   *   **  **   ***  ***
                                     *   ** */
    bool IsNakadeShape(const SgPointSet& area, int nuPoints);
    bool IsNakadeShape(const SgPointSet& area);

    /** Return true if a point can become an eye by adding one more
        defender's move. Same logic as CanMakeEye(), but optimized since
        it does not compute the eye-making move.
    */
    bool IsPossibleEye(const GoBoard& bd, SgBlackWhite color, SgPoint p);

    /** Check if point is a single point eye with one or two adjacent blocks.
        This is a fast eye detection routine, which can be used instead of
        Benson's static life detection, when end-of-game detection is a
        performance bottle-neck (e.g. for machine-learning or Monte Carlo).
        It detects single-point eyes surrounded by a single block or by two
        blocks that share another single point eye.
        Larger eyes can be reduced to simple eyes (assuming chinese rules,
        so that playing on does not change the score).
        @todo Add example to documentation where this method fails */
    bool IsSimpleEye(const GoBoard& bd, SgPoint p, SgBlackWhite c);

    /**  */
    bool IsSinglePointEye(const GoBoard& bd, SgPoint p, SgBlackWhite c);

    /** does playing at p create one of the standard nakade shapes? */
    template<class BOARD>
    bool MakesNakadeShape(const BOARD& bd, SgPoint p,
                                 SgBlackWhite toPlay);

    /** Return true if a point can become an eye by adding number of
        defender's move. */
    bool NumberOfMoveToEye(const GoBoard& bd, SgBlackWhite c, SgPoint p,
                           int& number);

    /** As IsSinglePointEye, but allows diagonal points to be eyes.
        Slightly slower, but identifies more single point eyes.
        E.g:
        @verbatim
        # X X X . .
        # O O X X X
        # O . O O X
        # . O . O X
        ###########
        @endverbatim */
    bool IsSinglePointEye2(const GoBoard& bd, SgPoint p, SgBlackWhite c);

    /** As IsSinglePointEye2, but specifying points assumed to be eyes. */
    bool IsSinglePointEye2(const GoBoard& bd, SgPoint p,
                           SgBlackWhite c, SgVector<SgPoint>& eyes);

    /** p is in a 2 point eye surrounded by a single chain */
    template<class BOARD>
    bool IsTwoPointEye(const BOARD& bd, SgPoint p,
                       SgBlackWhite c);

    /** As NumberOfMoveToEye2, but includes existing diagonal eyes,
        and allows opponent stones to be captured. */
    bool NumberOfMoveToEye2(const GoBoard& bd, SgBlackWhite c,
                            SgPoint p, int& nummoves);

    /** Count number of single point eyes for block p */
    int CountSinglePointEyes2(const GoBoard& bd, SgPoint p);

    /** Does block at p have two or more single point eyes? */
    bool SinglePointSafe2(const GoBoard& bd, SgPoint p);

    /** Does removing p split s into two or more parts? */
    bool IsSplitPt(SgPoint p, const SgPointSet& s);

    /** Does p locally, within a 3x3 region, split its neighbors in s?
        Even if the reply is 'yes', s might still be connected outside
        the region. */
    bool IsLocalSplitPt(SgPoint p, const SgPointSet& set);

    /** Area is tree shape if it does not contain a 2x2 square. */
    bool IsTreeShape(const SgPointSet& area);

    /** Vital point in small shape - usually has most liberties
        See implementation for details. */
    bool IsVitalPt(const SgPointSet& points, SgPoint p, SgBlackWhite opp,
               const GoBoard& bd);

    /** Analyze small region locally for number of eyes.
        color: the player surrounding the area.
        isNakade: only one eye
        makeNakade: attacker can reduce to one eye, defender can live locally.
        makeFalse: attacker can make the area into a false eye.
        maybeSeki, sureSeki: can there be, or is there, a seki between
            boundary stones and interior opponent stones?
        @todo: seki support is primitive only.
        vital is set iff makeNakade or makeFalse */
    void TestNakade(const SgPointSet& points, const GoBoard& bd,
                    SgBlackWhite color, bool isFullyEnclosed, bool& isNakade,
                    bool& makeNakade, bool& makeFalse, bool& maybeSeki,
                    bool& sureSeki, SgPoint* vital);

    bool CheckInterior(const GoBoard& bd, const SgPointSet& area,
                       SgBlackWhite opp, bool checkBlocks);
}

inline bool AreSameBlocks(const SgPoint anchors1[], const SgPoint anchors2[])
{
    int i = 0;
    for (; anchors1[i] != SG_ENDPOINT; ++i)
    {
        if (anchors2[i] == SG_ENDPOINT)
            return false;
        if (! GoBoardUtil::ContainsAnchor(anchors2, anchors1[i]))
            return false;
    }
    return anchors2[i] == SG_ENDPOINT;
}

/** Recognizes 2x2 block of points.
 Relies on the current implementation
 where SgSetIterator produces set members in sorted order,
 such that bulky four points have values p, p + WE, p + NS, p + WE + NS */
inline bool IsBulkyFour(const SgPointSet& points)
{
    SG_ASSERT(points.IsSize(4));
    SgSetIterator it(points);
    SgPoint p1 = *it;
    ++it;
    if (*it != p1 + SG_WE)
        return false;
    ++it;
    if (*it != p1 + SG_NS)
        return false;
    ++it;
    if (*it != p1 + SG_WE + SG_NS)
        return false;
    SG_ASSERT(GoEyeUtil::DegreeCode(points) == 400);
    return true;
}

inline bool IsTShape(const SgPointSet& block)
{
    return GoEyeUtil::DegreeCode(block) == 1030;
}

inline bool IsBulkyFive(const SgPointSet& block)
{
    return GoEyeUtil::DegreeCode(block) == 1310;
}

inline bool IsCross(const SgPointSet& block)
{
    return GoEyeUtil::DegreeCode(block) == 10040;
}

inline bool IsRabbitySix(const SgPointSet& block)
{
    return GoEyeUtil::DegreeCode(block) == 10320;
}

inline bool Is2x3Area(const SgPointSet& area)
{
    return GoEyeUtil::DegreeCode(area) == 2400;
}


template<class BOARD>
SgPoint GoEyeUtil::EmptyNeighbor(const BOARD& bd, SgPoint p)
{
    if (bd.IsEmpty(p + SG_NS))
        return p + SG_NS;
    if (bd.IsEmpty(p - SG_NS))
        return p - SG_NS;
    if (bd.IsEmpty(p + SG_WE))
        return p + SG_WE;
    if (bd.IsEmpty(p - SG_WE))
        return p - SG_WE;
    SG_ASSERT(false);
    return SG_NULLPOINT;
}

inline bool GoEyeUtil::IsNakadeShape(const SgPointSet& area, int nuPoints)
{
    switch (nuPoints)
    {
        case 1:
        case 2:
        case 3: return true;
        case 4: return IsBulkyFour(area) || IsTShape(area);
        case 5: return IsBulkyFive(area) || IsCross(area);
        case 6: return IsRabbitySix(area);
        default: // too big
            return false;
    }
}

inline bool GoEyeUtil::IsNakadeShape(const SgPointSet& area)
{
    return  IsNakadeShape(area, area.Size());
}

inline bool GoEyeUtil::IsSimpleEye(const GoBoard& bd, SgPoint p,
                                   SgBlackWhite c)
{
    // Function is inline despite its large size, because it returns quickly
    // on average, which makes the function call an overhead

    SgBlackWhite opp = SgOppBW(c);
    if (bd.HasEmptyNeighbors(p) || bd.HasNeighbors(p, opp))
        return false;
    SgArrayList<SgPoint,2> anchors;
    for (GoNbIterator it(bd, p); it; ++it)
    {
        SgPoint nbPoint = *it;
        SG_ASSERT(bd.IsColor(nbPoint, c));
        SgPoint nbAnchor = bd.Anchor(nbPoint);
        if (! anchors.Contains(nbAnchor))
        {
            if (anchors.Length() > 1)
                return false;
            anchors.PushBack(nbAnchor);
        }
    }
    if (anchors.Length() == 1)
        return true;
    for (GoBoard::LibertyIterator it(bd, anchors[0]); it; ++it)
    {
        SgPoint lib = *it;
        if (lib == p)
            continue;
        bool isSecondSharedEye = true;
        SgArrayList<SgPoint,2> foundAnchors;
        for (GoNbIterator it2(bd, lib); it2; ++it2)
        {
            SgPoint nbPoint = *it2;
            if (bd.GetColor(nbPoint) != c)
            {
                isSecondSharedEye = false;
                break;
            }
            SgPoint nbAnchor = bd.Anchor(nbPoint);
            if (! anchors.Contains(nbAnchor))
            {
                isSecondSharedEye = false;
                break;
            }
            if (! foundAnchors.Contains(nbAnchor))
                foundAnchors.PushBack(nbAnchor);
        }
        if (isSecondSharedEye && foundAnchors.Length() == 2)
            return true;
    }
    return false;
}

template<class BOARD>
bool GoEyeUtil::IsTwoPointEye(const BOARD& bd, SgPoint p, 
                   SgBlackWhite color)
{
    const SgBlackWhite opp = SgOppBW(color);
    if (bd.NumEmptyNeighbors(p) == 1
        && bd.NumNeighbors(p, opp) == 0
        )
    {
        const SgPoint p2 = GoBoardUtil::FindNeighbor(bd, p, SG_EMPTY);
        if (  bd.NumEmptyNeighbors(p2) == 1
           && bd.NumNeighbors(p2, opp) == 0
           )
        {
            // check if p, p2 are adjacent to the same blocks
            SgPoint nbanchorp[4 + 1];
            SgPoint nbanchorp2[4 + 1];
            bd.NeighborBlocks(p, color, nbanchorp);
            bd.NeighborBlocks(p2, color, nbanchorp2);
            SG_ASSERT(nbanchorp[0] != SG_ENDPOINT);
            SG_ASSERT(nbanchorp2[0] != SG_ENDPOINT);
            return ::AreSameBlocks(nbanchorp, nbanchorp2);
        }
    }
    return false;
}

template<class BOARD>
bool GoEyeUtil::MakesNakadeShape(const BOARD& bd, SgPoint move,
                                 SgBlackWhite toPlay)
{
    SgPointSet area;
    area.Include(move);
    int nu = 1;
    SgVector<SgPoint> toProcess;
    toProcess.PushBack(move);
    while (toProcess.NonEmpty())
    {
        SgPoint p = toProcess.Back();
        toProcess.PopBack();
        for (GoNb4Iterator<BOARD> it(bd, p); it; ++it)
            if (bd.IsColor(*it, toPlay) && ! area.Contains(*it))
            {
                area.Include(*it);
                toProcess.PushBack(*it);
                if (++nu > NAKADE_LIMIT)
                    return false;
            }
    }
    return IsNakadeShape(area, nu);
}

template<class BOARD>
bool GoEyeUtil::CanMakeEye(const BOARD& bd, SgBlackWhite color,
                           SgPoint p, SgPoint& move)
{
    bool isPossibleEye = false;
    SG_ASSERT(bd.GetColor(p) != color);
    const SgBlackWhite opp = SgOppBW(color);
    if (bd.Line(p) == 1) // corner or edge
    {
        const int nuOwn = (bd.Pos(p) == 1) ? 2 : 4;
        if (  bd.Num8Neighbors(p, color) == nuOwn
           && bd.Num8EmptyNeighbors(p) == 1
           )
        {
            isPossibleEye = true;
            move = GoBoardUtil::FindNeighbor(bd, p, SG_EMPTY);
        }
    }
    else // in center
    {
        // have all neighbors, and 2 diagonals, and can get a third
        if (  bd.NumNeighbors(p, color) == 4
           && bd.NumDiagonals(p, color) == 2
           && bd.NumEmptyDiagonals(p) > 0
           )
        {
            isPossibleEye = true;
            move = GoBoardUtil::FindDiagNeighbor(bd, p, SG_EMPTY);
        }
        // have 3 of 4 neighbors, can get the 4th, and have enough diagonals
        else if (  bd.NumNeighbors(p, color) == 3
                && bd.NumNeighbors(p, opp) == 0
                && bd.NumDiagonals(p, color) >= 3
                )
        {
            isPossibleEye = true;
            move = GoBoardUtil::FindNeighbor(bd, p, SG_EMPTY);
        }
    }
    
    return isPossibleEye;
}

//----------------------------------------------------------------------------

#endif // GO_EYEUTIL_H

