//----------------------------------------------------------------------------
/** @file GoUctPlayer.h
    Class GoUctPlayer. */
//----------------------------------------------------------------------------

#ifndef GOUCT_PLAYER_H
#define GOUCT_PLAYER_H

#include <boost/scoped_ptr.hpp>
#include <vector>
#include "GoBoard.h"
#include "GoBoardRestorer.h"
#include "GoPlayer.h"
#include "GoTimeControl.h"
#include "GoUctDefaultMoveFilter.h"
#include "GoUctFeatureKnowledge.h"
#include "GoUctGlobalSearch.h"
#include "GoUctObjectWithSearch.h"
#include "GoUctPlayoutPolicy.h"
#include "GoUctMoveFilter.h"
#include "SgArrayList.h"
#include "SgDebug.h"
#include "SgNbIterator.h"
#include "SgNode.h"
#include "SgPointArray.h"
#include "SgRestorer.h"
#include "SgMpiSynchronizer.h"
#include "SgTime.h"
#include "SgTimer.h"
#include "SgUctTreeUtil.h"
#include "SgWrite.h"

template<typename T,int SIZE> class SgSList;

//----------------------------------------------------------------------------

/** What search mode to use in GoUctPlayer to select a move. */
enum GoUctGlobalSearchMode
{
    /** No search, use playout policy to select a move. */
    GOUCT_SEARCHMODE_PLAYOUTPOLICY,

    /** Use UCT search. */
    GOUCT_SEARCHMODE_UCT,

    /** Do a 1-ply MC search. */
    GOUCT_SEARCHMODE_ONEPLY
};

//----------------------------------------------------------------------------

/** Player using UCT. */
template <class SEARCH, class THREAD>
class GoUctPlayer
    : public GoPlayer,
      public GoUctObjectWithSearch,
      public SgObjectWithDefaultTimeControl
{
public:
    /** Statistics collected by GoUctPlayer. */
    struct Statistics
    {
        std::size_t m_nuGenMove;

        SgStatisticsExt<float,std::size_t> m_reuse;

        SgStatisticsExt<double,std::size_t> m_gamesPerSecond;

        Statistics();

        void Clear();

        /** Write in human readable format. */
        void Write(std::ostream& out) const;
    };

    GoUctPlayoutPolicyParam m_playoutPolicyParam;

    GoUctDefaultMoveFilterParam m_rootFilterParam;

    GoUctDefaultMoveFilterParam m_treeFilterParam;

    GoUctFeatureKnowledgeParam m_featureParam;

    /** Constructor.
        @param bd The board. */
    GoUctPlayer(const GoBoard& bd);

    ~GoUctPlayer();


    /** @name Virtual functions of GoBoardSynchronizer */
    // @{

    void OnBoardChange();

    // @} // @name


    /** @name Virtual functions of GoPlayer */
    // @{

    SgPoint GenMove(const SgTimeRecord& time, SgBlackWhite toPlay);

    std::string Name() const;

    void Ponder();

    // @} // @name


    /** @name Virtual functions of SgObjectWithDefaultTimeControl */
    // @{

    SgDefaultTimeControl& TimeControl();

    const SgDefaultTimeControl& TimeControl() const;

    // @} // @name


    /** @name Virtual functions of GoUctObjectWithSearch */
    // @{

    GoUctSearch& Search();

    const GoUctSearch& Search() const;

    // @} // @name


    /** @name Parameters */
    // @{

    /** Automatically adapt the search parameters optimized for the current
        board size.
        If on, GoUctGlobalSearch::SetDefaultParameters will automatically
        be called, if the board size changes. */
    bool AutoParam() const;

    /** See AutoParam() */
    void SetAutoParam(bool enable);

    /** Pass early.
        Aborts search early, if value is above 1 - ResignThreshold(), and
        performs a second search to see, if it is still a win and all points
        are safe (using territory statistics) after playing a pass. If this
        is true, it plays a pass. */
    bool EarlyPass() const;

    /** See EarlyPass() */
    void SetEarlyPass(bool enable);

    /** Enforce opening moves in the corner on large boards.
        See GoUctUtil::GenForcedOpeningMove. Default is true. */
    bool ForcedOpeningMoves() const;

    /** See ForcedOpeningMoves() */
    void SetForcedOpeningMoves(bool enable);

    /** Ignore time settings of the game.
        Ignore time record given to GenMove() and only obeys maximum
        number of games and maximum time. Default is true. */
    bool IgnoreClock() const;

    /** See IgnoreClock() */
    void SetIgnoreClock(bool enable);

    /** Limit on number of simulated games per move. */
    SgUctValue MaxGames() const;

    /** See MaxGames() */
    void SetMaxGames(SgUctValue maxGames);

    /** Think during the opponents time.
        For enabling pondering, ReuseSubtree() also has to be true.
        Pondering search will be terminated after MaxGames() or
        MaxPonderTime(). */
    bool EnablePonder() const;

    /** See EnablePonder() */
    void SetEnablePonder(bool enable);

    /** Maximum ponder time in seconds.
        @see EnablePonder() */
    double MaxPonderTime() const;

    /** See MaxPonderTime() */
    void SetMaxPonderTime(double seconds);

    /** Minimum number of simulations to check for resign.
        This minimum number of simulations is also required to apply the
        early pass check (see EarlyPass()).
        Default is 3000. */
    SgUctValue ResignMinGames() const;

    /** See ResignMinGames()     */
    void SetResignMinGames(SgUctValue n);

    /** Use the root filter. */
    bool UseRootFilter() const;

    /** See UseRootFilter() */
    void SetUseRootFilter(bool enable);

    /** Reuse subtree from last search.
        Reuses the subtree from the last search, if the current position is
        a number of regular game moves later than the position that the
        previous search corresponds to. Default is true. */
    bool ReuseSubtree() const;

    /** See ReuseSubtree() */
    void SetReuseSubtree(bool enable);

    /** Threshold for position value to resign.
        Default is 0.01. */
    SgUctValue ResignThreshold() const;

    /** See ResignThreshold() */
    void SetResignThreshold(SgUctValue threshold);

    /** See GoUctGlobalSearchMode */
    GoUctGlobalSearchMode SearchMode() const;

    /** See GoUctGlobalSearchMode */
    void SetSearchMode(GoUctGlobalSearchMode mode);

    /** Print output of GoUctSearch. */
    bool WriteDebugOutput() const;

    /** See WriteDebugOutput() */
    void SetWriteDebugOutput(bool flag);

    // @} // @name


    /** @name Virtual functions of SgObjectWithDefaultTimeControl */
    // @{

    const Statistics& GetStatistics() const;

    void ClearStatistics();

    // @} // @name

    SEARCH& GlobalSearch();

    const SEARCH& GlobalSearch() const;

    /** Return the current root filter. */
    GoUctMoveFilter& RootFilter();

    /** Set a new root filter.
        Deletes the old root filter and takes ownership of the new filter. */
    void SetRootFilter(GoUctMoveFilter* filter);

    void SetMpiSynchronizer(const SgMpiSynchronizerHandle &synchronizerHandle);

    SgMpiSynchronizerHandle GetMpiSynchronizer();

 private:
    /** See GoUctGlobalSearchMode */
    GoUctGlobalSearchMode m_searchMode;

    /** See AutoParam() */
    bool m_autoParam;

    /** See ForcedOpeningMoves() */
    bool m_forcedOpeningMoves;

    /** See IgnoreClock() */
    bool m_ignoreClock;

    /** See EnablePonder() */
    bool m_enablePonder;

    /** See UseRootFilter() */
    bool m_useRootFilter;

    /** See ReuseSubtree() */
    bool m_reuseSubtree;

    /** See EarlyPass() */
    bool m_earlyPass;

    /** See ResignThreshold() */
    SgUctValue m_resignThreshold;

    const SgUctValue m_sureWinThreshold;

    /** Used in OnBoardChange() */
    int m_lastBoardSize;

    SgUctValue m_maxGames;

    SgUctValue m_resignMinGames;

    double m_maxPonderTime;

    SEARCH m_search;

    GoTimeControl m_timeControl;

    Statistics m_statistics;

    boost::scoped_ptr<GoUctMoveFilter> m_rootFilter;

    /** Playout policy used if search mode is
        GOUCT_SEARCHMODE_PLAYOUTPOLICY. */
    boost::scoped_ptr<GoUctPlayoutPolicy<GoBoard> > m_playoutPolicy;

    SgMpiSynchronizerHandle m_mpiSynchronizer;

    bool m_writeDebugOutput;

    SgMove GenMovePlayoutPolicy(SgBlackWhite toPlay);

    bool DoEarlyPassSearch(SgUctValue maxGames, double maxTime,
                           SgPoint searchMove, SgPoint& move);

    SgPoint DoSearch(SgBlackWhite toPlay, double maxTime,
                     bool isDuringPondering);

    void FindInitTree(SgUctTree& initTree, SgBlackWhite toPlay,
                      double maxTime);

    void SetDefaultParameters(int boardSize);

    bool VerifyNeutralMove(SgUctValue maxGames, double maxTime, SgPoint move);
};

template <class SEARCH, class THREAD>
inline bool GoUctPlayer<SEARCH, THREAD>::AutoParam() const
{
    return m_autoParam;
}

template <class SEARCH, class THREAD>
inline SEARCH&
GoUctPlayer<SEARCH, THREAD>::GlobalSearch()
{
    return m_search;
}

template <class SEARCH, class THREAD>
inline const SEARCH& GoUctPlayer<SEARCH, THREAD>::GlobalSearch() const
{
    return m_search;
}

template <class SEARCH, class THREAD>
inline bool GoUctPlayer<SEARCH, THREAD>::EarlyPass() const
{
    return m_earlyPass;
}

template <class SEARCH, class THREAD>
inline bool GoUctPlayer<SEARCH, THREAD>::EnablePonder() const
{
    return m_enablePonder;
}

template <class SEARCH, class THREAD>
inline bool GoUctPlayer<SEARCH, THREAD>::ForcedOpeningMoves() const
{
    return m_forcedOpeningMoves;
}

template <class SEARCH, class THREAD>
inline bool GoUctPlayer<SEARCH, THREAD>::IgnoreClock() const
{
    return m_ignoreClock;
}

template <class SEARCH, class THREAD>
inline SgUctValue GoUctPlayer<SEARCH, THREAD>::MaxGames() const
{
    return m_maxGames;
}

template <class SEARCH, class THREAD>
inline double GoUctPlayer<SEARCH, THREAD>::MaxPonderTime() const
{
    return m_maxPonderTime;
}

template <class SEARCH, class THREAD>
inline bool GoUctPlayer<SEARCH, THREAD>::UseRootFilter() const
{
    return m_useRootFilter;
}

template <class SEARCH, class THREAD>
inline SgUctValue GoUctPlayer<SEARCH, THREAD>::ResignMinGames() const
{
    return m_resignMinGames;
}

template <class SEARCH, class THREAD>
inline SgUctValue GoUctPlayer<SEARCH, THREAD>::ResignThreshold() const
{
    return m_resignThreshold;
}

template <class SEARCH, class THREAD>
inline bool GoUctPlayer<SEARCH, THREAD>::ReuseSubtree() const
{
    return m_reuseSubtree;
}

template <class SEARCH, class THREAD>
inline GoUctMoveFilter& GoUctPlayer<SEARCH, THREAD>::RootFilter()
{
    return *m_rootFilter;
}

template <class SEARCH, class THREAD>
inline GoUctGlobalSearchMode GoUctPlayer<SEARCH, THREAD>::SearchMode() const
{
    return m_searchMode;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetAutoParam(bool enable)
{
    m_autoParam = enable;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetEarlyPass(bool enable)
{
    m_earlyPass = enable;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetEnablePonder(bool enable)
{
    m_enablePonder = enable;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetForcedOpeningMoves(bool enable)
{
    m_forcedOpeningMoves = enable;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetIgnoreClock(bool enable)
{
    m_ignoreClock = enable;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetMaxGames(SgUctValue maxGames)
{
    m_maxGames = maxGames;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetMaxPonderTime(double seconds)
{
    m_maxPonderTime = seconds;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetUseRootFilter(bool enable)
{
    m_useRootFilter = enable;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetResignMinGames(SgUctValue n)
{
    m_resignMinGames = n;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetResignThreshold(SgUctValue threshold)
{
    m_resignThreshold = threshold;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetRootFilter(GoUctMoveFilter*
                                                       filter)
{
    m_rootFilter.reset(filter);
}

template <class SEARCH, class THREAD>
inline void 
GoUctPlayer<SEARCH, THREAD>::SetSearchMode(GoUctGlobalSearchMode mode)
{
    m_searchMode = mode;
}

template <class SEARCH, class THREAD>
inline void GoUctPlayer<SEARCH, THREAD>::SetMpiSynchronizer(const SgMpiSynchronizerHandle &handle)
{
    m_mpiSynchronizer = SgMpiSynchronizerHandle(handle);
    m_search.SetMpiSynchronizer(handle);
}

template <class SEARCH, class THREAD>
inline SgMpiSynchronizerHandle 
GoUctPlayer<SEARCH, THREAD>::GetMpiSynchronizer()
{
    return SgMpiSynchronizerHandle(m_mpiSynchronizer);
}

template <class SEARCH, class THREAD>
GoUctPlayer<SEARCH, THREAD>::Statistics::Statistics()
{
    Clear();
}

template <class SEARCH, class THREAD>
void GoUctPlayer<SEARCH, THREAD>::Statistics::Clear()
{
    m_nuGenMove = 0;
    m_gamesPerSecond.Clear();
    m_reuse.Clear();
}

template <class SEARCH, class THREAD>
bool GoUctPlayer<SEARCH, THREAD>::WriteDebugOutput() const
{
    return m_writeDebugOutput;
}

template <class SEARCH, class THREAD>
void GoUctPlayer<SEARCH, THREAD>::SetWriteDebugOutput(bool flag)
{
    m_writeDebugOutput = flag;
}

template <class SEARCH, class THREAD>
void GoUctPlayer<SEARCH, THREAD>::Statistics::Write(std::ostream& out) const
{
    out << SgWriteLabel("NuGenMove") << m_nuGenMove << '\n'
        << SgWriteLabel("GamesPerSec");
    m_gamesPerSecond.Write(out);
    out << '\n'
        << SgWriteLabel("Reuse");
    m_reuse.Write(out);
    out << '\n';
}

template <class SEARCH, class THREAD>
GoUctPlayer<SEARCH, THREAD>::GoUctPlayer(const GoBoard& bd)
    : GoPlayer(bd),
      m_searchMode(GOUCT_SEARCHMODE_UCT),
      m_autoParam(true),
      m_forcedOpeningMoves(true),
      m_ignoreClock(false),
      m_enablePonder(false),
      m_useRootFilter(true),
      m_reuseSubtree(true),
      m_earlyPass(true),
      m_sureWinThreshold(0.80f),
      m_lastBoardSize(-1),
      m_maxGames(std::numeric_limits<SgUctValue>::max()),
      m_resignMinGames(5000),
      m_maxPonderTime(300),
      m_search(
           Board(),
           new GoUctPlayoutPolicyFactory<GoUctBoard>(m_playoutPolicyParam),
           m_playoutPolicyParam,
           m_treeFilterParam,
           m_featureParam),
      m_timeControl(Board()),
      m_rootFilter(new GoUctDefaultMoveFilter(Board(), m_rootFilterParam)),
      m_mpiSynchronizer(SgMpiNullSynchronizer::Create()),
      m_writeDebugOutput(true)
{
    SetDefaultParameters(Board().Size());
    m_search.SetMpiSynchronizer(m_mpiSynchronizer);
    m_treeFilterParam.SetCheckSafety(false);
}

template <class SEARCH, class THREAD>
GoUctPlayer<SEARCH, THREAD>::~GoUctPlayer()
{ }

template <class SEARCH, class THREAD>
void GoUctPlayer<SEARCH, THREAD>::ClearStatistics()
{
    m_statistics.Clear();
}

typedef SgPointArray<SgUctStatistics> TerrArray;

inline bool HasStatsForAllMoves(const GoBoard& bd, const TerrArray& territory)
{
    for (GoBoard::Iterator it(bd); it; ++it)
        if (territory[*it].Count() == 0)
        {
            // No statistics, maybe all simulations aborted due to
            // max length or mercy rule.
            SgDebug() << "GoUctPlayer: no early pass possible (no stat) for"
                        << SgWritePoint(*it) << '\n';
            return false;
        }
    return true;
}

// translate from Black's view to player's view
inline SgUctValue ValueForPlayer(SgUctValue v, SgBlackWhite player)
{
    if (player == SG_WHITE)
        return 1 - v; // use InverseEstimate? Everywhere?
    else
        return v;
}
    
inline bool HasNonControlledLib(const GoBoard& bd,
                                SgPoint block,
                                SgBlackWhite toPlay,
                                const TerrArray& territory,
                                SgUctValue threshold)
{
    SG_ASSERT(bd.IsColor(block, toPlay));

    const SgUctValue blockMean = ValueForPlayer(territory[block].Mean(),
                                                toPlay);
    if (blockMean < threshold) // block not safe, probably dead. No fillin.
        // todo check for 1-threshold instead to check for dead blocks?
        // what happens if unsettled blocks passed in here?
        return false;

    for (GoBoard::LibertyIterator it(bd, block); it; ++it)
    {
        const SgUctValue mean = ValueForPlayer(territory[*it].Mean(), toPlay);
        if (mean < threshold)
        {
            SgDebug() << "non-controlled liberty " << SgWritePoint(*it)
            << " of block " << SgWritePoint(block)
            << " mean " << territory[*it].Mean()
            << "\n";

            return true;
        }
    }
    return false;
}

inline bool AllowFillinMove(const GoBoard& bd, SgPoint move,
                     const TerrArray& territory, SgUctValue threshold)
{
    /*  Idea: if adj. block has another liberty that is not controlled by us
        - neutral or controlled by opponent (e.g. our selfatari)
        then allow the move. Does not tactics-check it,
        assumes this is the search move. */
    SG_ASSERT(bd.IsEmpty(move));
    const SgBlackWhite toPlay = bd.ToPlay();
    for (GoNeighborBlockIterator it(bd, move, toPlay); it; ++it)
        if (HasNonControlledLib(bd, *it, toPlay, territory, threshold))
                return true;
    return false;
}

/** Perform a search after playing a pass and see if it is still a win and
    all points are safe as determined by territory statistics.
    @param maxGames Maximum simulations for the search
    @param maxTime Maximum time for the search
    @param searchMove A proposed move that is checked if early pass 
           is possible. If SG_PASS, the engine tries to find its own move.
    @param[out] move The move to play (pass or a neutral point to fill)
    @return @c true, if still winning and everything is safe after a pass */
template <class SEARCH, class THREAD>
bool GoUctPlayer<SEARCH, THREAD>::DoEarlyPassSearch(SgUctValue maxGames,
                                                    double maxTime,
                                                    SgPoint searchMove,
                                                    SgPoint& move)
{
    SgDebug() << "GoUctPlayer: doing a search if early pass is possible\n";
    GoBoard& bd = Board();
    bd.Play(SG_PASS);
    bool winAfterPass = false;
    bool passWins = GoBoardUtil::TrompTaylorPassWins(bd, bd.ToPlay());
    m_mpiSynchronizer->SynchronizePassWins(passWins);
    if (passWins)
    {
        // Using GoBoardUtil::TrompTaylorPassWins here is not strictly
        // necessary, but safer, because it can take the search in the
        // else-statement a while to explore the pass move
        winAfterPass = false;
    }
    else
    {
        SgRestorer<bool> restorer1(&m_search.m_param.m_territoryStatistics);
        SgRestorer<bool> restorer2(&m_search.m_param.m_mercyRule);
        m_search.m_param.m_territoryStatistics = true;
        // Mercy rule can prevent territory stats from being collected
        m_search.m_param.m_mercyRule = false; 
        std::vector<SgPoint> sequence;
        SgUctValue value = m_search.Search(maxGames, maxTime, sequence);
        value = m_search.InverseEstimate(value);
        winAfterPass = (value > m_sureWinThreshold);
    }
    bd.Undo();

    bool earlyPassPossible = true;
    if (earlyPassPossible && ! winAfterPass)
    {
        SgDebug() << "GoUctPlayer: no early pass possible (no win)\n";
        earlyPassPossible = false;
    }
    move = SG_PASS;
    THREAD& threadState = dynamic_cast<THREAD&>(m_search.ThreadState(0));
    const TerrArray territory = threadState.m_territoryStatistics;
    if (earlyPassPossible && ! HasStatsForAllMoves(bd, territory))
    {
        earlyPassPossible = false;
    }
    
    if (earlyPassPossible
        && searchMove != SG_PASS
        && AllowFillinMove(bd, searchMove, territory, m_sureWinThreshold))
    {
        move = searchMove;
        SgDebug() << "GoUctPlayer: allow fill-in move "
                    << SgWritePoint(move) << "\n";
        return true;
    }


    if (earlyPassPossible)
    {
        for (GoBoard::Iterator it(bd); it; ++it)
        {
            const SgUctValue mean = territory[*it].Mean();
            if (  mean > 1 - m_sureWinThreshold
               && mean < m_sureWinThreshold)
            {
                // Check if neutral point
                bool isSafeToPlayAdj = false;
                bool isSafeOppAdj = false;
                for (GoNbIterator it2(bd, *it); it2; ++it2)
                {
                    const SgUctValue nbMean = territory[*it2].Mean();
                    if (nbMean > m_sureWinThreshold)
                        isSafeToPlayAdj = true;
                    if (nbMean < 1 - m_sureWinThreshold)
                        isSafeOppAdj = true;
                }
                if (isSafeToPlayAdj && isSafeOppAdj)
                {
                    if (bd.IsLegal(*it) && ! GoBoardUtil::SelfAtari(bd, *it))
                        move = *it;
                    else
                    {
                        SgDebug() <<
                            "GoUctPlayer: no early pass possible"
                            " (neutral illegal or self-atari)\n";
                        earlyPassPossible = false;
                        break;
                    }
                }
                else
                {
                    SgDebug()
                    << "GoUctPlayer: no early pass possible (unsafe point "
                    << SgWritePoint(*it) << ")\n";
                    earlyPassPossible = false;
                    break;
                }
            }
        }
    }

    m_mpiSynchronizer->SynchronizeEarlyPassPossible(earlyPassPossible);
    if (! earlyPassPossible)
        return false;
    m_mpiSynchronizer->SynchronizeMove(move);
    if (move == SG_PASS)
        SgDebug() << "GoUctPlayer: early pass is possible\n";
    else if (VerifyNeutralMove(maxGames, maxTime, move))
        SgDebug() << "GoUctPlayer: generate play on neutral point\n";
    else
    {
        SgDebug() << "GoUctPlayer: neutral move failed to verify\n";
        return false;
    }
    return true;
}

/** Run the search for a given color.
    @param toPlay
    @param maxTime
    @param isDuringPondering Hint that search is done during pondering (this
    handles the decision to discard an aborted FindInitTree differently)
    @return The best move or SG_NULLMOVE if terminal position (can also
    happen, if @c isDuringPondering, no search was performed, because
    DoSearch() was aborted during FindInitTree()). */
template <class SEARCH, class THREAD>
SgPoint GoUctPlayer<SEARCH, THREAD>::DoSearch(SgBlackWhite toPlay, 
                                              double maxTime,
                                              bool isDuringPondering)
{
    SgUctTree* initTree = 0;
    SgTimer timer;
    double timeInitTree = 0;
    if (m_reuseSubtree)
    {
        initTree = &m_search.GetTempTree();
        timeInitTree = -timer.GetTime();
        FindInitTree(*initTree, toPlay, maxTime);
        timeInitTree += timer.GetTime();
        if (isDuringPondering)
        {
            bool aborted = SgUserAbort();
            m_mpiSynchronizer->SynchronizeUserAbort(aborted);
            if (aborted)
            // If abort occurs during pondering, better don't start a search
            // with a truncated init tree. The search would be aborted after
            // one game anyway, because it also checks SgUserAbort(). There is
            // a higher chance to reuse a larger part of the current tree in
            // the next regular move search.
            return SG_NULLMOVE;
        }
    }
    std::vector<SgMove> rootFilter;
    double timeRootFilter = 0;
    if (m_useRootFilter)
    {
        timeRootFilter = -timer.GetTime();
        rootFilter = m_rootFilter->Get();
        timeRootFilter += timer.GetTime();
    }
    maxTime -= timer.GetTime();
    m_search.SetToPlay(toPlay);
    std::vector<SgPoint> sequence;
    SgUctEarlyAbortParam earlyAbort;
    earlyAbort.m_threshold = m_sureWinThreshold;
    earlyAbort.m_minGames = m_resignMinGames;
    earlyAbort.m_reductionFactor = 3;
    SgUctValue value = m_search.Search(m_maxGames, maxTime, sequence, rootFilter,
                                  initTree, &earlyAbort);

    bool wasEarlyAbort = m_search.WasEarlyAbort();
    SgUctValue rootMoveCount = m_search.Tree().Root().MoveCount();
    m_mpiSynchronizer->SynchronizeSearchStatus(value, wasEarlyAbort, rootMoveCount);

    if (m_writeDebugOutput)
    {
        // Write debug output to a string stream first to avoid intermingling
        // of debug output with response in GoGui GTP shell
        std::ostringstream out;
        m_search.WriteStatistics(out);
        out << SgWriteLabel("Value") << std::fixed << std::setprecision(2) 
            << value << '\n' << SgWriteLabel("Sequence") 
            << SgWritePointList(sequence, "", false);
        if (m_reuseSubtree)
            out << SgWriteLabel("TimeInitTree") << std::fixed 
                << std::setprecision(2) << timeInitTree << '\n';
        if (m_useRootFilter)
            out << SgWriteLabel("TimeRootFilter") << std::fixed 
                << std::setprecision(2) << timeRootFilter << '\n';
        SgDebug() << out.str();
    }

    if (  value < m_resignThreshold
       && rootMoveCount > m_resignMinGames
       )
        return SG_RESIGN;

    SgPoint move;
    if (sequence.empty())
        move = SG_PASS;
    else
    {
        move = *(sequence.begin());
        move = GoUctSearchUtil::TrompTaylorPassCheck(move, m_search);
    }

    // If SgUctSearch aborted early, use the remaining time/nodes for doing a
    // search, if an early pass is possible
    if (m_earlyPass && (wasEarlyAbort || value > m_sureWinThreshold))
    {
        maxTime -= timer.GetTime();
        SgPoint earlyPassMove;
        if (DoEarlyPassSearch(m_maxGames / earlyAbort.m_reductionFactor,
                              maxTime, move, earlyPassMove))
            move = earlyPassMove;
    }

    m_mpiSynchronizer->SynchronizeMove(move);
    return move;
}

/** Find initial tree for search, if subtree reusing is enabled.
    Goes back in the tree until the node is found, the search tree is valid
    for and checks if the path of nodes corresponds to an alternating
    sequence of moves starting with the color to play of the search tree.
    @see SetReuseSubtree */
template <class SEARCH, class THREAD>
void GoUctPlayer<SEARCH, THREAD>::FindInitTree(SgUctTree& initTree, 
                                               SgBlackWhite toPlay,
                                               double maxTime)
{
    Board().SetToPlay(toPlay);
    std::vector<SgPoint> sequence;
    if (! m_search.BoardHistory().SequenceToCurrent(Board(), sequence))
    {
        SgDebug() << "GoUctPlayer: No tree to reuse found\n";
        return;
    }
    SgUctTreeUtil::ExtractSubtree(m_search.Tree(), initTree, sequence, true,
                                  maxTime, m_search.PruneMinCount());
    const size_t initTreeNodes = initTree.NuNodes();
    const size_t oldTreeNodes = m_search.Tree().NuNodes();
    if (oldTreeNodes > 1 && initTreeNodes >= 1)
    {
        const float reuse = float(initTreeNodes) / float(oldTreeNodes);
        const int reusePercent = static_cast<int>(100 * reuse);
        SgDebug() << "GoUctPlayer: Reusing " << initTreeNodes
                  << " nodes (" << reusePercent << "%)\n";

        //SgDebug() << SgWritePointList(sequence, "Sequence", false);
        m_statistics.m_reuse.Add(reuse);
    }
    else
    {
        SgDebug() << "GoUctPlayer: Subtree to reuse has 0 nodes\n";
        m_statistics.m_reuse.Add(0.f);
    }

    // Check consistency
    if (initTree.Root().HasChildren())
    {
        for (SgUctChildIterator it(initTree, initTree.Root()); it; ++it)
            if (! Board().IsLegal((*it).Move()))
            {
                SgWarning() <<
                    "GoUctPlayer: illegal move in root child of init tree\n";
                initTree.Clear();
                // Should not happen, if no bugs
                SG_ASSERT(false);
            }
    }
}

template <class SEARCH, class THREAD>
SgPoint GoUctPlayer<SEARCH, THREAD>::GenMove(const SgTimeRecord& time,
                                             SgBlackWhite toPlay)
{
    ++m_statistics.m_nuGenMove;
    if (m_searchMode == GOUCT_SEARCHMODE_PLAYOUTPOLICY)
        return GenMovePlayoutPolicy(toPlay);
    const GoBoard& bd = Board();
    SgMove move = SG_NULLMOVE;
    if (m_forcedOpeningMoves)
    {
        move = GoUctUtil::GenForcedOpeningMove(bd);
        if (move != SG_NULLMOVE)
            SgDebug() << "GoUctPlayer: Forced opening move\n";
    }
    if (move == SG_NULLMOVE && GoBoardUtil::TrompTaylorPassWins(bd, toPlay))
    {
        move = SG_PASS;
        SgDebug() << "GoUctPlayer: Pass wins (Tromp-Taylor rules)\n";
    }
    if (move == SG_NULLMOVE)
    {
        double maxTime;
        if (m_ignoreClock)
            maxTime = std::numeric_limits<double>::max();
        else
            maxTime = m_timeControl.TimeForCurrentMove(time,
                                                       ! m_writeDebugOutput);
        if (m_searchMode == GOUCT_SEARCHMODE_ONEPLY)
        {
            m_search.SetToPlay(toPlay);
            SgUctValue ignoreValue;
            move = m_search.SearchOnePly(m_maxGames, maxTime, ignoreValue);
            if (move == SG_NULLMOVE)
                move = SG_PASS;
        }
        else
        {
            SG_ASSERT(m_searchMode == GOUCT_SEARCHMODE_UCT);
            move = DoSearch(toPlay, maxTime, false);
            m_statistics.m_gamesPerSecond.Add(
                                      m_search.Statistics().m_gamesPerSecond);
        }
    }
    return move;
}

template <class SEARCH, class THREAD>
SgMove GoUctPlayer<SEARCH, THREAD>::GenMovePlayoutPolicy(SgBlackWhite toPlay)
{
    GoBoard& bd = Board();
    GoBoardRestorer restorer(bd);
    bd.SetToPlay(toPlay);
    if (m_playoutPolicy.get() == 0)
        m_playoutPolicy.reset(
            new GoUctPlayoutPolicy<GoBoard>(bd, m_playoutPolicyParam));
    m_playoutPolicy->StartPlayout();
    SgPoint move = m_playoutPolicy->GenerateMove();
    m_playoutPolicy->EndPlayout();
    if (move == SG_NULLMOVE)
    {
        SgDebug() <<
            "GoUctPlayer: GoUctPlayoutPolicy generated SG_NULLMOVE\n";
        return SG_PASS;
    }
    return move;
}

template <class SEARCH, class THREAD>
const typename GoUctPlayer<SEARCH, THREAD>::Statistics& 
GoUctPlayer<SEARCH, THREAD>::GetStatistics() const
{
    return m_statistics;
}

template <class SEARCH, class THREAD>
std::string GoUctPlayer<SEARCH, THREAD>::Name() const
{
    return "GoUctPlayer";
}

template <class SEARCH, class THREAD>
void GoUctPlayer<SEARCH, THREAD>::OnBoardChange()
{
    int size = Board().Size();
    if (m_autoParam && size != m_lastBoardSize)
    {
        SgDebug() << "GoUctPlayer: Setting default parameters for size "
                  << size << '\n';
        SetDefaultParameters(size);
        m_search.SetDefaultParameters(size);
        m_lastBoardSize = size;
    }
}

template <class SEARCH, class THREAD>
void GoUctPlayer<SEARCH, THREAD>::Ponder()
{
    const GoBoard& bd = Board();
    if (! m_enablePonder || m_searchMode != GOUCT_SEARCHMODE_UCT)
        return;
    if (! m_reuseSubtree)
    {
        // Don't ponder, wouldn't use the result in the next GenMove
        // anyway if reuseSubtree is not enabled
        SgWarning() << "Pondering needs reuse_subtree enabled.\n";
        return;
    }
    SgDebug() << "GoUctPlayer::Ponder: start\n";
    DoSearch(bd.ToPlay(), m_maxPonderTime, true);
    SgDebug() << "GoUctPlayer::Ponder: end\n";
}

template <class SEARCH, class THREAD>
GoUctSearch& GoUctPlayer<SEARCH, THREAD>::Search()
{
    return m_search;
}

template <class SEARCH, class THREAD>
const GoUctSearch& GoUctPlayer<SEARCH, THREAD>::Search() const
{
    return m_search;
}

template <class SEARCH, class THREAD>
void GoUctPlayer<SEARCH, THREAD>::SetDefaultParameters(int boardSize)
{
    m_timeControl.SetFastOpenMoves(0);
    m_timeControl.SetMinTime(0);
    m_timeControl.SetRemainingConstant(0.5);
    if (boardSize < 15)
    {
        m_resignThreshold = SgUctValue(0.05);
    }
    else
    {
        // Need higher resign threshold, because GoUctGlobalSearch uses
        // length modification on large board
        m_resignThreshold = SgUctValue(0.08);
    }
}

template <class SEARCH, class THREAD>
void GoUctPlayer<SEARCH, THREAD>::SetReuseSubtree(bool enable)
{
    m_reuseSubtree = enable;
}

template <class SEARCH, class THREAD>
SgDefaultTimeControl& GoUctPlayer<SEARCH, THREAD>::TimeControl()
{
    return m_timeControl;
}

template <class SEARCH, class THREAD>
const SgDefaultTimeControl& GoUctPlayer<SEARCH, THREAD>::TimeControl() const
{
    return m_timeControl;
}

/** Verify that the move selected by DoEarlyPassSearch is viable.
    Prevent blunders from so-called neutral moves that are not. */
template <class SEARCH, class THREAD>
bool GoUctPlayer<SEARCH, THREAD>::VerifyNeutralMove(SgUctValue maxGames, 
                                                    double maxTime,
                                                    SgPoint move)
{
    GoBoard& bd = Board();
    bd.Play(move);
    std::vector<SgPoint> sequence;
    SgUctValue value = m_search.Search(maxGames, maxTime, sequence);
    value = m_search.InverseEstimate(value);
    bd.Undo();
    return value >= m_sureWinThreshold;
}

//----------------------------------------------------------------------------

#endif // GOUCT_PLAYER_H
