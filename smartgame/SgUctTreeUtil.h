//----------------------------------------------------------------------------
/** @file SgUctTreeUtil.h
    Utility functions for users of SgUctSearch.h */
//----------------------------------------------------------------------------

#ifndef SG_UCTTREEUTIL_H
#define SG_UCTTREEUTIL_H

#include <cstddef>
#include <iosfwd>
#include "SgUctValue.h"
#include "SgStatistics.h"

class SgUctNode;
class SgUctTree;

//----------------------------------------------------------------------------

/** Statistical properties of a SgUctTree.
    @ingroup sguctgroup */
class SgUctTreeStatistics
{
public:
    /** See m_posCounts */
    static const std::size_t MAX_MOVECOUNT = 5;

    std::size_t m_nuNodes;

    /** Number of nodes that have a certain move count. */
    std::size_t m_moveCounts[MAX_MOVECOUNT];

    /** Difference between move value and RAVE value. */
    SgStatisticsExt<SgUctValue,std::size_t> m_biasRave;

    SgUctTreeStatistics();

    void Clear();

    /** Clear statistics and traverse tree to compute statistics. */
    void Compute(const SgUctTree& tree);

    void Write(std::ostream& out) const;
};

/** Write a SgUctTreeStatistics.
    @relatesalso SgUctTreeStatistics */
std::ostream& operator<<(std::ostream& out, const SgUctTreeStatistics& stat);

//----------------------------------------------------------------------------

/** Utility functions for users of SgUctTree.
    @ingroup sguctgroup */
namespace SgUctTreeUtil
{
    /** Extract the subtree from the node after a sequence of moves.
        The result is an empty tree, if the sequence of moves does
        not correspond to a sequence of nodes from the root node in the
        given tree.
        @param tree The source tree.
        @param[out] target The target tree (will be cleared before using it).
        @param sequence The sequence of moves.
        @param warnTruncate See SgUctTree::ExtractSubtree
        @param maxTime See SgUctTree::ExtractSubtree
        @param minCount */
    void ExtractSubtree(const SgUctTree& tree, SgUctTree& target,
                        const std::vector<SgMove>& sequence,
                        bool warnTruncate,
                        double maxTime = std::numeric_limits<double>::max(),
                        SgUctValue minCount = 0);

    /** Find child of node corresponding to a given move.
        @return The child node, or 0 if node has no such child. */
    const SgUctNode* FindChildWithMove(const SgUctTree& tree,
                                       const SgUctNode& node, SgMove move);

    /** Replay move sequence from root of tree and return the resulting node.
        Return 0 if no node found at any stage */
    const SgUctNode* FindMatchingNode(const SgUctTree& tree,
                                      const std::vector<SgMove>& sequence);
} // namespace SgUctTreeUtil

//----------------------------------------------------------------------------

#endif // SG_UCTTREEUTIL_H
