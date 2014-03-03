//----------------------------------------------------------------------------
/** @file GoBoardUtilTest.cpp
    Unit tests for GoBoardUtil. */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "GoBoardUtil.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "GoSetup.h"
#include "GoSetupUtil.h"
#include "SgPointSet.h"

using namespace GoBoardUtil;
using SgPointUtil::Pt;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(GoBlockIteratorTest)
{
    GoSetup setup;
    setup.AddBlack(Pt(1, 1));
    setup.AddBlack(Pt(1, 2));
    setup.AddWhite(Pt(2, 1));
    setup.AddWhite(Pt(2, 2));
    setup.AddBlack(Pt(3, 7));
    setup.AddWhite(Pt(SG_MAX_SIZE, SG_MAX_SIZE));
    GoBoard bd(SG_MAX_SIZE, setup);
    GoBlockIterator it(bd);
    BOOST_CHECK_EQUAL(*it, Pt(1, 1));
    BOOST_CHECK(it);
    ++it;
    BOOST_CHECK_EQUAL(*it, Pt(2, 1));
    BOOST_CHECK(it);
    ++it;
    BOOST_CHECK_EQUAL(*it, Pt(3, 7));
    BOOST_CHECK(it);
    ++it;
    BOOST_CHECK_EQUAL(*it, Pt(SG_MAX_SIZE, SG_MAX_SIZE));
    BOOST_CHECK(it);
    ++it;
    BOOST_CHECK(! it);
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_AddWall)
{
    if (SG_MAX_SIZE >= 19)
    {
        GoBoard bd(19);
        AddWall(bd, SG_BLACK, Pt(3, 3), 10, SG_NS);
        BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_BLACK), 10);
        BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_WHITE), 0);
        AddWall(bd, SG_WHITE, Pt(4,3), 5, SG_NS + SG_WE);
        BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_BLACK), 10);
        BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_WHITE), 5);
        AddWall(bd, SG_WHITE, Pt(5,3), 4, SG_NS + 2 * SG_WE);
        BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_BLACK), 10);
        BOOST_CHECK_EQUAL(bd.TotalNumStones(SG_WHITE), 9);
    }
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_AdjacentBlocks_1)
{
    // . . . . .
    // @ O O @ .
    // O O . @ .
    GoSetup setup;
    setup.AddBlack(Pt(1, 2));
    setup.AddBlack(Pt(4, 1));
    setup.AddBlack(Pt(4, 2));
    setup.AddWhite(Pt(1, 1));
    setup.AddWhite(Pt(2, 1));
    setup.AddWhite(Pt(2, 2));
    setup.AddWhite(Pt(3, 2));
    GoBoard bd(9, setup);
    SgVector<SgPoint> blocks;
    AdjacentBlocks(bd, Pt(2, 1), 10, &blocks);
    BOOST_CHECK_EQUAL(blocks.Length(), 2);
    BOOST_CHECK(blocks.Contains(Pt(1, 2)));
    BOOST_CHECK(blocks.Contains(Pt(4, 1)));
    AdjacentBlocks(bd, Pt(2, 1), 1, &blocks);
    BOOST_CHECK_EQUAL(blocks.Length(), 1);
    BOOST_CHECK(blocks.Contains(Pt(1, 2)));
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_AdjacentBlocks_2)
{
    GoSetup setup;
    // . . . . .
    // @ O O @ .
    // O O @ @ .
    setup.AddWhite(Pt(1, 1));
    setup.AddWhite(Pt(2, 1));
    setup.AddWhite(Pt(2, 2));
    setup.AddWhite(Pt(3, 2));
    setup.AddBlack(Pt(1, 2));
    setup.AddBlack(Pt(3, 1));
    setup.AddBlack(Pt(4, 1));
    setup.AddBlack(Pt(4, 2));
    GoBoard bd(9, setup);
    SgVector<SgPoint> blocks;
    AdjacentBlocks(bd, Pt(2, 1), 10, &blocks);
    BOOST_CHECK_EQUAL(blocks.Length(), 2);
    BOOST_CHECK(blocks.Contains(Pt(1, 2)));
    BOOST_CHECK(blocks.Contains(Pt(3, 1)));
    AdjacentBlocks(bd, Pt(2, 1), 1, &blocks);
    BOOST_CHECK_EQUAL(blocks.Length(), 1);
    BOOST_CHECK(blocks.Contains(Pt(1, 2)));
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_AllLegalMoves)
{
    GoBoard bd(5);
    GoPointList moves(AllLegalMoves(bd));
    BOOST_CHECK_EQUAL(moves.Length(), 25);
    bd.Play(Pt(1,2));
    GoPointList moves2(AllLegalMoves(bd));
    BOOST_CHECK_EQUAL(moves2.Length(), 24);
    bd.Play(SG_PASS);
    GoPointList moves3(AllLegalMoves(bd));
    BOOST_CHECK_EQUAL(moves3.Length(), 24);
    bd.Play(Pt(2,1));
    GoPointList moves4(AllLegalMoves(bd));
    BOOST_CHECK_EQUAL(moves4.Length(), 22); // Pt(1,1) illegal for White
    bd.Play(SG_PASS);
    GoPointList moves5(AllLegalMoves(bd));
    BOOST_CHECK_EQUAL(moves5.Length(), 23); // Pt(1,1) legal for Black
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_AllLegalMoves_Ko)
{
    // |OX
    // |.OX
    // ----
    GoBoard bd(5);
    bd.Play(Pt(2,2));
    bd.Play(Pt(1,2));
    bd.Play(Pt(3,1));
    bd.Play(Pt(2,1));
    GoPointList moves(AllLegalMoves(bd));
    BOOST_CHECK_EQUAL(bd.AllEmpty().Size(), 21);
    BOOST_CHECK_EQUAL(moves.Length(), 21);
    bd.Play(Pt(1,1)); // capture
    BOOST_CHECK_EQUAL(bd.AllEmpty().Size(), 21);
    GoPointList moves2(AllLegalMoves(bd));
    BOOST_CHECK_EQUAL(moves2.Length(), 20);
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_DiagonalsOfColor)
{
    GoSetup setup;
    setup.AddBlack(Pt(1, 1));
    setup.AddWhite(Pt(2, 1));
    setup.AddWhite(Pt(1, 2));
    GoBoard bd(9, setup);
    SgVector<SgPoint> diags;
    DiagonalsOfColor(bd, Pt(1, 1), SG_BLACK, &diags);
    BOOST_CHECK_EQUAL(diags.Length(), 0);
    DiagonalsOfColor(bd, Pt(1, 1), SG_WHITE, &diags);
    BOOST_CHECK_EQUAL(diags.Length(), 0);
    DiagonalsOfColor(bd, Pt(1, 1), SG_EMPTY, &diags);
    BOOST_CHECK_EQUAL(diags.Length(), 1);
    BOOST_CHECK(diags.Contains(Pt(2, 2)));
    DiagonalsOfColor(bd, Pt(2, 2), SG_BLACK, &diags);
    BOOST_CHECK_EQUAL(diags.Length(), 1);
    BOOST_CHECK(diags.Contains(Pt(1, 1)));
    DiagonalsOfColor(bd, Pt(2, 2), SG_WHITE, &diags);
    BOOST_CHECK_EQUAL(diags.Length(), 0);
    DiagonalsOfColor(bd, Pt(2, 2), SG_EMPTY, &diags);
    BOOST_CHECK_EQUAL(diags.Length(), 3);
    BOOST_CHECK(diags.Contains(Pt(3, 1)));
    BOOST_CHECK(diags.Contains(Pt(1, 3)));
    BOOST_CHECK(diags.Contains(Pt(3, 3)));
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_GainsLiberties)
{
    std::string s("XO..O.\n"
                  ".XOO..\n"
                  "......\n"
                  "......\n"
                  "......\n"
                  "......");
    int boardSize;
    GoSetup setup = GoSetupUtil::CreateSetupFromString(s, boardSize);
    setup.m_player = SG_BLACK;
    GoBoard bd(boardSize, setup);

    BOOST_CHECK(GainsLiberties(bd, Pt(1,6), Pt(1,5)));
    BOOST_CHECK(! GainsLiberties(bd, Pt(2,5), Pt(1,5)));
    BOOST_CHECK(GainsLiberties(bd, Pt(2,5), Pt(2,4)));
    BOOST_CHECK(GainsLiberties(bd, Pt(2,6), Pt(3,6)));

    const SgPoint anchor1 = bd.Anchor( Pt(3,5));
    BOOST_CHECK(! GainsLiberties(bd, anchor1, Pt(3,6)));
    BOOST_CHECK(GainsLiberties(bd, anchor1, Pt(3,4)));
    BOOST_CHECK(GainsLiberties(bd, anchor1, Pt(4,4)));
    BOOST_CHECK(! GainsLiberties(bd, anchor1, Pt(4,6)));
    BOOST_CHECK(GainsLiberties(bd, anchor1, Pt(5,5)));

    BOOST_CHECK(GainsLiberties(bd, Pt(5,6), Pt(4,6)));
    BOOST_CHECK(GainsLiberties(bd, Pt(5,6), Pt(5,5)));
    BOOST_CHECK(! GainsLiberties(bd, Pt(5,6), Pt(6,6)));
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_GetDirtyRegion)
{
    SgRect s1, s2, l1, l2;
    GoBoard bd(9);

    // ..XO
    // .XOX
    // ..X.
    // ....
    // ....

    s1 = GetDirtyRegion(bd, Pt(3, 3), SG_BLACK, false, true);
    l1 = GetDirtyRegion(bd, Pt(3, 3), SG_BLACK, true, true);
    bd.Play(Pt(3, 3), SG_BLACK);
    s2 = GetDirtyRegion(bd, Pt(3, 3), SG_BLACK, false, false);
    l2 = GetDirtyRegion(bd, Pt(3, 3), SG_BLACK, true, false);
    BOOST_CHECK_EQUAL(s1, SgRect(3, 3, 3, 3));
    BOOST_CHECK_EQUAL(l1, SgRect(3, 3, 3, 3));
    BOOST_CHECK_EQUAL(s2, SgRect(3, 3, 3, 3));
    BOOST_CHECK_EQUAL(l2, SgRect(3, 3, 3, 3));

    s1 = GetDirtyRegion(bd, Pt(3, 4), SG_WHITE, false, true);
    l1 = GetDirtyRegion(bd, Pt(3, 4), SG_WHITE, true, true);
    bd.Play(Pt(3, 4), SG_WHITE);
    s2 = GetDirtyRegion(bd, Pt(3, 4), SG_WHITE, false, false);
    l2 = GetDirtyRegion(bd, Pt(3, 4), SG_WHITE, true, false);
    BOOST_CHECK_EQUAL(s1, SgRect(3, 3, 4, 4));
    BOOST_CHECK_EQUAL(l1, SgRect(3, 3, 3, 4));
    BOOST_CHECK_EQUAL(s2, SgRect(3, 3, 4, 4));
    BOOST_CHECK_EQUAL(l2, SgRect(3, 3, 3, 4));

    s1 = GetDirtyRegion(bd, Pt(4, 4), SG_BLACK, false, true);
    l1 = GetDirtyRegion(bd, Pt(4, 4), SG_BLACK, true, true);
    bd.Play(Pt(4, 4), SG_BLACK);
    s2 = GetDirtyRegion(bd, Pt(4, 4), SG_BLACK, false, false);
    l2 = GetDirtyRegion(bd, Pt(4, 4), SG_BLACK, true, false);
    BOOST_CHECK_EQUAL(s1, SgRect(4, 4, 4, 4));
    BOOST_CHECK_EQUAL(l1, SgRect(3, 4, 4, 4));
    BOOST_CHECK_EQUAL(s2, SgRect(4, 4, 4, 4));
    BOOST_CHECK_EQUAL(l2, SgRect(3, 4, 4, 4));

    s1 = GetDirtyRegion(bd, Pt(4, 5), SG_WHITE, false, true);
    l1 = GetDirtyRegion(bd, Pt(4, 5), SG_WHITE, true, true);
    bd.Play(Pt(4, 5), SG_WHITE);
    s2 = GetDirtyRegion(bd, Pt(4, 5), SG_WHITE, false, false);
    l2 = GetDirtyRegion(bd, Pt(4, 5), SG_WHITE, true, false);
    BOOST_CHECK_EQUAL(s1, SgRect(4, 4, 5, 5));
    BOOST_CHECK_EQUAL(l1, SgRect(4, 4, 4, 5));
    BOOST_CHECK_EQUAL(s2, SgRect(4, 4, 5, 5));
    BOOST_CHECK_EQUAL(l2, SgRect(4, 4, 4, 5));

    s1 = GetDirtyRegion(bd, Pt(2, 4), SG_BLACK, false, true);
    l1 = GetDirtyRegion(bd, Pt(2, 4), SG_BLACK, true, true);
    bd.Play(Pt(2, 4), SG_BLACK);
    s2 = GetDirtyRegion(bd, Pt(2, 4), SG_BLACK, false, false);
    l2 = GetDirtyRegion(bd, Pt(2, 4), SG_BLACK, true, false);
    BOOST_CHECK_EQUAL(s1, SgRect(2, 2, 4, 4));
    BOOST_CHECK_EQUAL(l1, SgRect(2, 3, 4, 4));
    BOOST_CHECK_EQUAL(s2, SgRect(2, 2, 4, 4));
    BOOST_CHECK_EQUAL(l2, SgRect(2, 3, 4, 4));

    s1 = GetDirtyRegion(bd, SG_PASS, SG_WHITE, false, true);
    l1 = GetDirtyRegion(bd, SG_PASS, SG_WHITE, true, true);
    bd.Play(SG_PASS, SG_WHITE);
    s2 = GetDirtyRegion(bd, SG_PASS, SG_WHITE, false, false);
    l2 = GetDirtyRegion(bd, SG_PASS, SG_WHITE, true, false);
    BOOST_CHECK_EQUAL(s1, SgRect());
    BOOST_CHECK_EQUAL(l1, SgRect());
    BOOST_CHECK_EQUAL(s2, SgRect());
    BOOST_CHECK_EQUAL(l2, SgRect());

    s1 = GetDirtyRegion(bd, Pt(3, 5), SG_BLACK, false, true);
    l1 = GetDirtyRegion(bd, Pt(3, 5), SG_BLACK, true, true);
    bd.Play(Pt(3, 5), SG_BLACK);
    s2 = GetDirtyRegion(bd, Pt(3, 5), SG_BLACK, false, false);
    l2 = GetDirtyRegion(bd, Pt(3, 5), SG_BLACK, true, false);
    BOOST_CHECK_EQUAL(s1, SgRect(3, 3, 4, 5));
    BOOST_CHECK_EQUAL(l1, SgRect(2, 4, 3, 5));
    BOOST_CHECK_EQUAL(s2, SgRect(3, 3, 4, 5));
    BOOST_CHECK_EQUAL(l2, SgRect(2, 4, 3, 5));
}


void TestNb(const GoBoard& bd, SgPoint p, int nuNb)
{
    int nu = 0;
    for (GoNbIterator it(bd, p); it; ++it)
        ++nu;
    BOOST_CHECK_EQUAL(nu, nuNb);
}

void TestSize(int size)
{
    GoBoard bd(size);
    TestNb(bd, Pt(1,1), 2);
    TestNb(bd, Pt(1,size), 2);
    TestNb(bd, Pt(size,1), 2);
    TestNb(bd, Pt(size,size), 2);
    TestNb(bd, Pt(1,2), 3);
    TestNb(bd, Pt(size - 1,1), 3);
    TestNb(bd, Pt(size,size - 1), 3);
    TestNb(bd, Pt(2,2), 4);
    TestNb(bd, Pt(size - 1,size - 1), 4);
    if (size > 3)
    {
        TestNb(bd, Pt(3, size), 3);
        TestNb(bd, Pt(3, size - 2), 4);
    }
}
    
BOOST_AUTO_TEST_CASE(GoBoardUtilTest_GoNbIterator)
{
    TestSize(3);
    TestSize(5);
    if (SG_MAX_SIZE >= 9)
        TestSize(9);
    if (SG_MAX_SIZE >= 13)
        TestSize(13);
    if (SG_MAX_SIZE >= 19)
        TestSize(19);
}
    
BOOST_AUTO_TEST_CASE(GoBoardUtilTest_HasAdjacentBlocks)
{
    GoSetup setup;
    setup.AddBlack(Pt(1, 1));
    setup.AddBlack(Pt(2, 1));
    GoBoard bd(9, setup);
    BOOST_CHECK(! HasAdjacentBlocks(bd, Pt(2, 1), 2));
    setup.AddWhite(Pt(1, 2));
    bd.Init(9, setup);
    BOOST_CHECK(! HasAdjacentBlocks(bd, Pt(2, 1), 1));
    BOOST_CHECK(HasAdjacentBlocks(bd, Pt(2, 1), 2));
    BOOST_CHECK(HasAdjacentBlocks(bd, Pt(2, 1), 3));
}

/* @todo
 add tests for
    bool GoBoardUtil::PointHasAdjacentBlock(const GoBoard& bd, SgPoint p,
                                            SgBlackWhite color, int maxLib)
*/

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_IsSimpleChain_1)
{
    std::string s("X...XOOX.\n"
                  ".X..X..X.\n"
                  "....XOOX.\n"
                  ".........\n"
                  "..X...X..\n"
                  "O.OX.XO.O\n"
                  "........O\n"
                  "..O...OOO\n"
                  ".........");
    int boardSize;
    GoSetup setup = GoSetupUtil::CreateSetupFromString(s, boardSize);
    GoBoard bd(boardSize, setup);
    SgPoint other;
    BOOST_CHECK(IsSimpleChain(bd, Pt(1, 9), other));
    BOOST_CHECK_EQUAL(Pt(2, 8), other);
    BOOST_CHECK(IsSimpleChain(bd, Pt(6, 9), other));
    BOOST_CHECK_EQUAL(Pt(6, 7), other);
    BOOST_CHECK(! IsSimpleChain(bd, Pt(3, 4), other));
    BOOST_CHECK(IsSimpleChain(bd, Pt(7, 4), other));
    BOOST_CHECK_EQUAL(Pt(7, 2), other);
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_IsSnapback_1)
{
    GoSetup setup;
    setup.AddBlack(Pt(2, 2));
    setup.AddBlack(Pt(3, 1));
    setup.AddWhite(Pt(1, 2));
    setup.AddWhite(Pt(2, 1));
    GoBoard bd(9, setup);
    BOOST_CHECK(! IsSnapback(bd, Pt(2, 1)));
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_IsSnapback_2)
{
    GoSetup setup;
    setup.AddBlack(Pt(3, 2));
    setup.AddBlack(Pt(4, 1));
    setup.AddBlack(Pt(1, 1));
    setup.AddWhite(Pt(1, 2));
    setup.AddWhite(Pt(2, 2));
    setup.AddWhite(Pt(3, 1));
    GoBoard bd(9, setup);
    BOOST_CHECK(IsSnapback(bd, Pt(3, 1)));
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_KeepsOrGainsLiberties)
{
    std::string s("XO..O.\n"
                  ".XOO..\n"
                  "......\n"
                  "......\n"
                  "......\n"
                  "......");
    int boardSize;
    GoSetup setup = GoSetupUtil::CreateSetupFromString(s, boardSize);
    setup.m_player = SG_BLACK;
    GoBoard bd(boardSize, setup);

    BOOST_CHECK(KeepsOrGainsLiberties(bd, Pt(1,6), Pt(1,5)));
    BOOST_CHECK(KeepsOrGainsLiberties(bd, Pt(2,5), Pt(1,5)));
    BOOST_CHECK(KeepsOrGainsLiberties(bd, Pt(2,5), Pt(2,4)));
    BOOST_CHECK(KeepsOrGainsLiberties(bd, Pt(2,6), Pt(3,6)));

    const SgPoint anchor1 = bd.Anchor( Pt(3,5));
    BOOST_CHECK(! KeepsOrGainsLiberties(bd, anchor1, Pt(3,6)));
    BOOST_CHECK(KeepsOrGainsLiberties(bd, anchor1, Pt(3,4)));
    BOOST_CHECK(KeepsOrGainsLiberties(bd, anchor1, Pt(4,4)));
    BOOST_CHECK(KeepsOrGainsLiberties(bd, anchor1, Pt(4,6)));
    BOOST_CHECK(KeepsOrGainsLiberties(bd, anchor1, Pt(5,5)));

    BOOST_CHECK(KeepsOrGainsLiberties(bd, Pt(5,6), Pt(4,6)));
    BOOST_CHECK(KeepsOrGainsLiberties(bd, Pt(5,6), Pt(5,5)));
    BOOST_CHECK(KeepsOrGainsLiberties(bd, Pt(5,6), Pt(6,6)));
}

/** Test GoBoardUtil::NeighborsOfColor */
BOOST_AUTO_TEST_CASE(GoBoardUtilTest_NeighborsOfColor)
{
    GoSetup setup;
    setup.AddBlack(Pt(1, 1));
    setup.AddWhite(Pt(2, 1));
    setup.AddWhite(Pt(1, 2));
    GoBoard bd(9, setup);
    SgArrayList<SgPoint,4> neighbors;
    neighbors = NeighborsOfColor(bd, Pt(1, 1), SG_BLACK);
    BOOST_CHECK_EQUAL(neighbors.Length(), 0);
    neighbors = NeighborsOfColor(bd, Pt(1, 1), SG_WHITE);
    BOOST_CHECK_EQUAL(neighbors.Length(), 2);
    BOOST_CHECK(neighbors.Contains(Pt(2, 1)));
    BOOST_CHECK(neighbors.Contains(Pt(1, 2)));
    neighbors = NeighborsOfColor(bd, Pt(1, 1), SG_EMPTY);
    BOOST_CHECK_EQUAL(neighbors.Length(), 0);
    neighbors = NeighborsOfColor(bd, Pt(2, 2), SG_BLACK);
    BOOST_CHECK_EQUAL(neighbors.Length(), 0);
    neighbors = NeighborsOfColor(bd, Pt(2, 2), SG_WHITE);
    BOOST_CHECK_EQUAL(neighbors.Length(), 2);
    BOOST_CHECK(neighbors.Contains(Pt(2, 1)));
    BOOST_CHECK(neighbors.Contains(Pt(1, 2)));
    neighbors = NeighborsOfColor(bd, Pt(2, 2), SG_EMPTY);
    BOOST_CHECK_EQUAL(neighbors.Length(), 2);
    BOOST_CHECK(neighbors.Contains(Pt(3, 2)));
    BOOST_CHECK(neighbors.Contains(Pt(2, 3)));
}

/** Test GoBoardUtil::NeighborsOfColor (SgVector version) */
BOOST_AUTO_TEST_CASE(GoBoardUtilTest_NeighborsOfColor_SgVector)
{
    GoSetup setup;
    setup.AddBlack(Pt(1, 1));
    setup.AddWhite(Pt(2, 1));
    setup.AddWhite(Pt(1, 2));
    GoBoard bd(9, setup);
    SgVector<SgPoint> neighbors;
    NeighborsOfColor(bd, Pt(1, 1), SG_BLACK, &neighbors);
    BOOST_CHECK_EQUAL(neighbors.Length(), 0);
    NeighborsOfColor(bd, Pt(1, 1), SG_WHITE, &neighbors);
    BOOST_CHECK_EQUAL(neighbors.Length(), 2);
    BOOST_CHECK(neighbors.Contains(Pt(2, 1)));
    BOOST_CHECK(neighbors.Contains(Pt(1, 2)));
    NeighborsOfColor(bd, Pt(1, 1), SG_EMPTY, &neighbors);
    BOOST_CHECK_EQUAL(neighbors.Length(), 0);
    NeighborsOfColor(bd, Pt(2, 2), SG_BLACK, &neighbors);
    BOOST_CHECK_EQUAL(neighbors.Length(), 0);
    NeighborsOfColor(bd, Pt(2, 2), SG_WHITE, &neighbors);
    BOOST_CHECK_EQUAL(neighbors.Length(), 2);
    BOOST_CHECK(neighbors.Contains(Pt(2, 1)));
    BOOST_CHECK(neighbors.Contains(Pt(1, 2)));
    NeighborsOfColor(bd, Pt(2, 2), SG_EMPTY, &neighbors);
    BOOST_CHECK_EQUAL(neighbors.Length(), 2);
    BOOST_CHECK(neighbors.Contains(Pt(3, 2)));
    BOOST_CHECK(neighbors.Contains(Pt(2, 3)));
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_OtherLiberty)
{
    std::string s("XO.XO.\n"
                  ".XOO..\n"
                  "..XX..\n"
                  "......\n"
                  "......\n"
                  "X...OX");
    int boardSize;
    GoSetup setup = GoSetupUtil::CreateSetupFromString(s, boardSize);
    setup.m_player = SG_BLACK;
    GoBoard bd(boardSize, setup);
    {
        const SgPoint anchor = bd.Anchor(Pt(1,1));
        const SgPoint lib1 = Pt(1,2);
        const SgPoint lib2 = Pt(2,1);
        BOOST_CHECK_EQUAL(OtherLiberty(bd, anchor, lib1), lib2);
        BOOST_CHECK_EQUAL(OtherLiberty(bd, anchor, lib2), lib1);
    }
    {
        const SgPoint anchor = bd.Anchor(Pt(5,1));
        const SgPoint lib1 = Pt(4,1);
        const SgPoint lib2 = Pt(5,2);
        BOOST_CHECK_EQUAL(OtherLiberty(bd, anchor, lib1), lib2);
        BOOST_CHECK_EQUAL(OtherLiberty(bd, anchor, lib2), lib1);
    }
    {
        const SgPoint anchor = bd.Anchor(Pt(3,5));
        const SgPoint lib1 = Pt(3,6);
        const SgPoint lib2 = Pt(5,5);
        BOOST_CHECK_EQUAL(OtherLiberty(bd, anchor, lib1), lib2);
        BOOST_CHECK_EQUAL(OtherLiberty(bd, anchor, lib2), lib1);
    }
    {
        const SgPoint anchor = bd.Anchor(Pt(5,6));
        const SgPoint lib1 = Pt(5,5);
        const SgPoint lib2 = Pt(6,6);
        BOOST_CHECK_EQUAL(OtherLiberty(bd, anchor, lib1), lib2);
        BOOST_CHECK_EQUAL(OtherLiberty(bd, anchor, lib2), lib1);
    }
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_ReduceToAnchors)
{
    std::string s("XO.\n"
                  "XX.\n"
                  ".OO");
    int boardSize;
    GoSetup setup = GoSetupUtil::CreateSetupFromString(s, boardSize);
    GoBoard bd(boardSize, setup);
    SgPointSet black = bd.All(SG_BLACK);
    BOOST_CHECK_EQUAL(black.Size(), 3);
    SgPointSet white = bd.All(SG_WHITE);
    BOOST_CHECK_EQUAL(white.Size(), 3);
        
    // SgVector version
    {
    SgVector<SgPoint> stones;
    black.ToVector(&stones);
    GoBoardUtil::ReduceToAnchors(bd, &stones);
    BOOST_CHECK_EQUAL(stones.Length(), 1);
    stones.Clear();
    white.ToVector(&stones);
    GoBoardUtil::ReduceToAnchors(bd, &stones);
    BOOST_CHECK_EQUAL(stones.Length(), 2);
    }
}

void CheckSelfAtari(const GoBoard& bd, SgPoint p, int nuExpectedStones)
{
    // check both versions of SelfAtari
    BOOST_CHECK(GoBoardUtil::SelfAtari(bd, p));
    int nuStones = 0;
    BOOST_CHECK(GoBoardUtil::SelfAtari(bd, p, nuStones));
    BOOST_CHECK_EQUAL(nuStones, nuExpectedStones);
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_SelfAtari_1)
{
    // . @ . O O . O O . 9
    // @ @ @ @ @ . @ @ @ 8
    // O . O . @ . @ @ @ 7
    // @ @ @ @ . @ O . O 6
    // O O . @ . @ O . O 5
    // O O . @ . . @ @ @ 4
    // @ @ . . . . + . . 3
    // . . O O . . @ . . 2
    // . O . @ . . . @ . 1
    // 1 2 3 4 5 6 7 8 9
    GoSetup setup;
    setup.AddBlack(Pt(4, 1));
    setup.AddBlack(Pt(8, 1));
    setup.AddBlack(Pt(7, 2));
    setup.AddBlack(Pt(1, 3));
    setup.AddBlack(Pt(2, 3));
    setup.AddBlack(Pt(4, 4));
    setup.AddBlack(Pt(7, 4));
    setup.AddBlack(Pt(8, 4));
    setup.AddBlack(Pt(9, 4));
    setup.AddBlack(Pt(4, 5));
    setup.AddBlack(Pt(6, 5));
    setup.AddBlack(Pt(1, 6));
    setup.AddBlack(Pt(2, 6));
    setup.AddBlack(Pt(3, 6));
    setup.AddBlack(Pt(4, 6));
    setup.AddBlack(Pt(6, 6));
    setup.AddBlack(Pt(5, 7));
    setup.AddBlack(Pt(7, 7));
    setup.AddBlack(Pt(8, 7));
    setup.AddBlack(Pt(9, 7));
    setup.AddBlack(Pt(1, 8));
    setup.AddBlack(Pt(2, 8));
    setup.AddBlack(Pt(3, 8));
    setup.AddBlack(Pt(4, 8));
    setup.AddBlack(Pt(5, 8));
    setup.AddBlack(Pt(7, 8));
    setup.AddBlack(Pt(8, 8));
    setup.AddBlack(Pt(9, 8));
    setup.AddBlack(Pt(2, 9));

    setup.AddWhite(Pt(2, 1));
    setup.AddWhite(Pt(3, 2));
    setup.AddWhite(Pt(4, 2));
    setup.AddWhite(Pt(1, 4));
    setup.AddWhite(Pt(2, 4));
    setup.AddWhite(Pt(1, 5));
    setup.AddWhite(Pt(2, 5));
    setup.AddWhite(Pt(7, 5));
    setup.AddWhite(Pt(9, 5));
    setup.AddWhite(Pt(7, 6));
    setup.AddWhite(Pt(9, 6));
    setup.AddWhite(Pt(1, 7));
    setup.AddWhite(Pt(3, 7));
    setup.AddWhite(Pt(4, 9));
    setup.AddWhite(Pt(5, 9));
    setup.AddWhite(Pt(7, 9));
    setup.AddWhite(Pt(8, 9));
    
    GoBoard bd(9, setup);

    bd.SetToPlay(SG_BLACK);
    CheckSelfAtari(bd, Pt(1, 1), 1);
    CheckSelfAtari(bd, Pt(3, 1), 2);
    CheckSelfAtari(bd, Pt(6, 9), 1);
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(2, 2)));
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(5, 1)));
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(9, 1)));
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(1, 9)));

    bd.SetToPlay(SG_WHITE);
    CheckSelfAtari(bd, Pt(7, 1), 1);
    CheckSelfAtari(bd, Pt(9, 1), 1);
    CheckSelfAtari(bd, Pt(3, 5), 5);
    CheckSelfAtari(bd, Pt(8, 5), 5);
    CheckSelfAtari(bd, Pt(8, 6), 5);
    CheckSelfAtari(bd, Pt(2, 7), 3);
    CheckSelfAtari(bd, Pt(4, 7), 2);
    CheckSelfAtari(bd, Pt(3, 9), 3);
    CheckSelfAtari(bd, Pt(9, 9), 3);
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(5, 1)));
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(3, 1)));
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(1, 1)));
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(3, 4)));
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(6, 9)));
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(6, 8)));
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_SelfAtari_2)
{
    // ko and capture - recapture cases
    // . O @ . . . . . . 9
    // O @ . . . @ @ O . 8
    // @ . . . @ O . . O 7
    // . . O @ . @ @ O . 6
    // . O @ . @ O . . . 5
    // . . O @ . @ @ O . 4
    // . . . . . . . . . 3
    // @ O . . . . . O @ 2
    // . @ O . . . O . O 1
    // 1 2 3 4 5 6 7 8 9

    GoSetup setup;
    setup.AddBlack(Pt(2, 1));
    setup.AddBlack(Pt(1, 2));
    setup.AddBlack(Pt(9, 2));
    setup.AddBlack(Pt(4, 4));
    setup.AddBlack(Pt(6, 4));
    setup.AddBlack(Pt(7, 4));
    setup.AddBlack(Pt(3, 5));
    setup.AddBlack(Pt(5, 5));
    setup.AddBlack(Pt(4, 6));
    setup.AddBlack(Pt(6, 6));
    setup.AddBlack(Pt(7, 6));
    setup.AddBlack(Pt(1, 7));
    setup.AddBlack(Pt(5, 7));
    setup.AddBlack(Pt(2, 8));
    setup.AddBlack(Pt(6, 8));
    setup.AddBlack(Pt(7, 8));
    setup.AddBlack(Pt(3, 9));
    
    setup.AddWhite(Pt(3, 1));
    setup.AddWhite(Pt(7, 1));
    setup.AddWhite(Pt(9, 1));
    setup.AddWhite(Pt(2, 2));
    setup.AddWhite(Pt(8, 2));
    setup.AddWhite(Pt(3, 4));
    setup.AddWhite(Pt(8, 4));
    setup.AddWhite(Pt(2, 5));
    setup.AddWhite(Pt(6, 5));
    setup.AddWhite(Pt(3, 6));
    setup.AddWhite(Pt(8, 6));
    setup.AddWhite(Pt(6, 7));
    setup.AddWhite(Pt(9, 7));
    setup.AddWhite(Pt(1, 8));
    setup.AddWhite(Pt(8, 8));
    setup.AddWhite(Pt(2, 9));
    
    GoBoard bd(9, setup);

    bd.SetToPlay(SG_BLACK);
    CheckSelfAtari(bd, Pt(1, 1), 3);
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(8, 1))); // ko
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(1, 9))); // capture 2

    bd.SetToPlay(SG_WHITE);
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(1, 1))); // ko
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(4, 5))); // ko
    CheckSelfAtari(bd, Pt(7, 5), 2);
    CheckSelfAtari(bd, Pt(7, 7), 2); // recapture situation
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_SelfAtari_3)
{
    // sacrifice moves
    // O . @ O . O @ . . 9
    // @ @ @ O . O @ @ @ 8
    // O O O . . O O O O 7
    // @ O . . . . @ @ @ 6
    // @ O . . . . @ O O 5
    // . @ . . . . @ O O 4
    // @ . . . . . @ @ O 3
    // . . . . . . . O . 2
    // . . . . . . . . O 1
    // 1 2 3 4 5 6 7 8 9

    GoSetup setup;
    setup.AddBlack(Pt(1, 3));
    setup.AddBlack(Pt(7, 3));
    setup.AddBlack(Pt(8, 3));
    setup.AddBlack(Pt(2, 4));
    setup.AddBlack(Pt(7, 4));
    setup.AddBlack(Pt(1, 5));
    setup.AddBlack(Pt(7, 5));
    setup.AddBlack(Pt(1, 6));
    setup.AddBlack(Pt(7, 6));
    setup.AddBlack(Pt(8, 6));
    setup.AddBlack(Pt(9, 6));
    setup.AddBlack(Pt(1, 8));
    setup.AddBlack(Pt(2, 8));
    setup.AddBlack(Pt(3, 8));
    setup.AddBlack(Pt(7, 8));
    setup.AddBlack(Pt(8, 8));
    setup.AddBlack(Pt(9, 8));
    setup.AddBlack(Pt(3, 9));
    setup.AddBlack(Pt(7, 9));
    
    setup.AddWhite(Pt(9, 1));
    setup.AddWhite(Pt(8, 2));
    setup.AddWhite(Pt(9, 3));
    setup.AddWhite(Pt(8, 4));
    setup.AddWhite(Pt(9, 4));
    setup.AddWhite(Pt(2, 5));
    setup.AddWhite(Pt(8, 5));
    setup.AddWhite(Pt(9, 5));
    setup.AddWhite(Pt(2, 6));
    setup.AddWhite(Pt(1, 7));
    setup.AddWhite(Pt(2, 7));
    setup.AddWhite(Pt(3, 7));
    setup.AddWhite(Pt(6, 7));
    setup.AddWhite(Pt(7, 7));
    setup.AddWhite(Pt(8, 7));
    setup.AddWhite(Pt(9, 7));
    setup.AddWhite(Pt(4, 8));
    setup.AddWhite(Pt(6, 8));
    setup.AddWhite(Pt(1, 9));
    setup.AddWhite(Pt(4, 9));
    setup.AddWhite(Pt(6, 9));
    
    GoBoard bd(9, setup);

    bd.SetToPlay(SG_BLACK);
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(9, 2))); // allow capture
    CheckSelfAtari(bd, Pt(2, 9), 5); // capture 1 within eye, still in atari
    CheckSelfAtari(bd, Pt(8, 9), 5);
    CheckSelfAtari(bd, Pt(9, 9), 5);
    bd.SetToPlay(SG_WHITE);
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(1, 4))); // allow capture
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(9, 2)));
    BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(2, 9)));
    //BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(8, 9))); // atari on opp.
    //BOOST_CHECK(! GoBoardUtil::SelfAtari(bd, Pt(9, 9))); // atari on opp.
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_TrompTaylorPassWins)
{
    GoBoard bd(9);
    GoRules& rules = bd.Rules();
    rules.SetKomi(6.5);
    rules.SetCaptureDead(true);
    rules.SetJapaneseScoring(false);
    bd.Play(Pt(5, 5), SG_BLACK);
    BOOST_CHECK(! TrompTaylorPassWins(bd, SG_BLACK)); // Last move was not a pass
    bd.Play(SG_PASS, SG_WHITE);
    BOOST_CHECK(TrompTaylorPassWins(bd, SG_BLACK));
    bd.Play(SG_PASS, SG_BLACK);
    BOOST_CHECK(! TrompTaylorPassWins(bd, SG_BLACK)); // Black is not to play
    BOOST_CHECK(! TrompTaylorPassWins(bd, SG_WHITE)); // Negative score for White
    bd.Undo();
    BOOST_CHECK(TrompTaylorPassWins(bd, SG_BLACK));
    rules.SetCaptureDead(false);
    BOOST_CHECK(! TrompTaylorPassWins(bd, SG_BLACK)); // Not Tromp-taylor rules
    rules.SetCaptureDead(true);
    BOOST_CHECK(TrompTaylorPassWins(bd, SG_BLACK));
    rules.SetJapaneseScoring(true);
    BOOST_CHECK(! TrompTaylorPassWins(bd, SG_BLACK)); // Not Tromp-taylor rules
}
    
BOOST_AUTO_TEST_CASE(GoBoardUtilTest_TrompTaylorScore)
{
    // . . . . . . . . .
    // . . . . . . . . .
    // . . + . . . + . .
    // . . . . . . . . .
    // . . . . . . . . .
    // O O O . . . . . .
    // . . O . . . + . .
    // @ @ O . . . . O O
    // . @ O . . . . O .
    GoSetup setup;
    setup.AddBlack(Pt(1, 2));
    setup.AddBlack(Pt(2, 1));
    setup.AddBlack(Pt(2, 2));
    setup.AddWhite(Pt(3, 1));
    setup.AddWhite(Pt(3, 2));
    setup.AddWhite(Pt(3, 3));
    setup.AddWhite(Pt(3, 4));
    setup.AddWhite(Pt(1, 4));
    setup.AddWhite(Pt(2, 4));
    setup.AddWhite(Pt(8, 1));
    setup.AddWhite(Pt(8, 2));
    setup.AddWhite(Pt(9, 2));
    GoBoard bd(9, setup);
    float komi = 6.5;
    BOOST_CHECK_CLOSE(GoBoardUtil::TrompTaylorScore(bd, komi), -77.5f, 1e-4f);
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_TrompTaylorScore_Empty9)
{
    GoBoard bd(9);
    float komi = 6.5;
    BOOST_CHECK_CLOSE(GoBoardUtil::TrompTaylorScore(bd, komi), -6.5f, 1e-4f);
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_TrompTaylorScore_Empty19)
{
    if (SG_MAX_SIZE >= 19)
    {
        GoBoard bd(19);
        float komi = 6.5;
        BOOST_CHECK_CLOSE(GoBoardUtil::TrompTaylorScore(bd, komi), 
                          -6.5f, 1e-4f);
    }
}

BOOST_AUTO_TEST_CASE(GoBoardUtilTest_TwoPasses)
{
    GoBoard bd(9);
    BOOST_CHECK(! GoBoardUtil::TwoPasses(bd));
    bd.Play(Pt(1, 1), SG_BLACK);
    BOOST_CHECK(! GoBoardUtil::TwoPasses(bd));
    bd.Play(SG_PASS, SG_WHITE);
    BOOST_CHECK(! GoBoardUtil::TwoPasses(bd));
    bd.Play(Pt(2, 2), SG_BLACK);
    BOOST_CHECK(! GoBoardUtil::TwoPasses(bd));
    bd.Play(SG_PASS, SG_WHITE);
    BOOST_CHECK(! GoBoardUtil::TwoPasses(bd));
    bd.Play(SG_PASS, SG_WHITE);
    // 2 passes by same color
    BOOST_CHECK(! GoBoardUtil::TwoPasses(bd));
    bd.Play(SG_PASS, SG_BLACK);
    BOOST_CHECK(GoBoardUtil::TwoPasses(bd));
    bd.SetToPlay(SG_BLACK);
    // wrong color order of pass moves wrt. current color to play
    BOOST_CHECK(! GoBoardUtil::TwoPasses(bd));
    bd.Undo();
    BOOST_CHECK(! GoBoardUtil::TwoPasses(bd));
}

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(GoRestoreKoRuleTest)
{
    GoBoard bd;
    bd.Rules().SetKoRule(GoRules::SIMPLEKO);
    {
        GoRestoreKoRule restoreKoRule(bd);
        bd.Rules().SetKoRule(GoRules::SUPERKO);
    }
    BOOST_CHECK_EQUAL(bd.Rules().GetKoRule(), GoRules::SIMPLEKO);
}

//----------------------------------------------------------------------------

} // namespace

