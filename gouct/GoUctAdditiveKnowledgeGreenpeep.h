//----------------------------------------------------------------------------
/** @file GoUctAdditiveKnowledgeGreenpeep.h */
//----------------------------------------------------------------------------

#ifndef GOUCT_ADDITIVEKNOWLEDGEGREENPEEP_H
#define GOUCT_ADDITIVEKNOWLEDGEGREENPEEP_H

#include "GoUctAdditiveKnowledge.h"
#include "GoUctPlayoutPolicy.h"
#include <boost/static_assert.hpp>


/* max 26-bit: 16-bit 8-neighbor core, 8-bit liberty & 2-away extension, 
	1 bit "ko exists", 1 bit defensive move */
const int NUMPATTERNS9X9 = 1<<26;

/* 24-bit: 16-bit 8-neighbor core, 8-bit liberty & 2-away extension */
const int NUMPATTERNS19X19 = 1<<24;

/** Types of Greenpeep knowledge patterns */
enum GoUctGreenpeepPatternType
{
    /** 9x9 patterns. These are used for board sizes < 15 */
    GOUCT_GREENPEEPPATTERNTYPE_9x9,
    /** 19x19 patterns. These are used for board sizes >= 15 */
    GOUCT_GREENPEEPPATTERNTYPE_19x19
};

//----------------------------------------------------------------------------

class GoUctAdditiveKnowledgeParamGreenpeep: public GoUctAdditiveKnowledgeParam
{
private:
public:
    GoUctAdditiveKnowledgeParamGreenpeep(GoUctGreenpeepPatternType patternType);
    ~GoUctAdditiveKnowledgeParamGreenpeep();

    int m_predictorArraySize;
    unsigned short* m_predictor;

    // These functions modify the static variables below, but they do not use
    // locking. These functions may therefore be called only when search
    // threads are not running, i.e. at a time when it is guaranteed that
    // GetParamObject() is NOT called.
    static void IncrementReferenceCount();
    static void DecrementReferenceCount();
    static void OnBoardSizeChange(const GoBoard& bd);
    // This function is called while search threads are running. It accesses
    // the static parameter object variables below, but it uses no locking.
    static const GoUctAdditiveKnowledgeParamGreenpeep* GetParamObject(GoUctGreenpeepPatternType patternType);

private:
    static int referenceCount;
    static SgGrid boardSize;
    static GoUctAdditiveKnowledgeParamGreenpeep* paramObject9x9;
    static GoUctAdditiveKnowledgeParamGreenpeep* paramObject19x19;

    static void UpdateParamObjects();
    static bool ShouldExistParamObject(GoUctGreenpeepPatternType patternType);
};

/** Use Greenpeep-style pattern values to make predictions. */
class GoUctAdditiveKnowledgeGreenpeep : public GoUctAdditiveKnowledge
{
public:
    GoUctAdditiveKnowledgeGreenpeep(const GoBoard& bd);
    ~GoUctAdditiveKnowledgeGreenpeep();

    /** The minimum value allowed by this predictor */
    SgUctValue Minimum() const;

    bool ProbabilityBased() const;

    void ProcessPosition(std::vector<SgUctMoveInfo>& moves);

    /** Print a pattern given its pattern code. 
    	3 typical examples:

        Example 1: the all empty pattern. @b shows its black turn (all patterns
        are black turn). Because the four neighbor points of @b are empty, the
        pattern shows the 4 points one step further in the four directions.
        They are all empty in this example.
                [  ]
            [  ][  ][  ]
        [  ][  ][@b][  ][  ]
            [  ][  ][  ]
                [  ]

        Example 2: this pattern has only 12 points.
        Since b3, west of @b, is taken by black, the extra 2 bits are now 
        used to encode the number of liberties (3) of this black block.

        [##], two steps to the north, encodes the border.
            [##]
        [  ][  ][b ]
        [b3][@b][  ][b ]
        [b ][  ][b ]
            [w ]

        Example 3: an edge pattern. It is also a capture since w1 is a
        white block with 1 liberty.
            [##][b1][b ]
        [##][##][@b][w1]
            [##][  ][b ]
                [  ]
    */
    static void PrintContext(unsigned int context, std::ostream& o);

    /** The scaling factor for this predictor */
    SgUctValue Scale() const;

private:
    unsigned int m_contexts[SG_MAX_ONBOARD + 1];
};

//----------------------------------------------------------------------------

inline SgUctValue GoUctAdditiveKnowledgeGreenpeep::Minimum() const
{
	return ProbabilityBased() ? 0.0001f : 0.05f;
}

inline bool GoUctAdditiveKnowledgeGreenpeep::ProbabilityBased() const
{
	return Board().Size() >= 15;
}

inline SgUctValue GoUctAdditiveKnowledgeGreenpeep::Scale() const
{
	return 0.03f;
}

//----------------------------------------------------------------------------

#endif // GOUCT_ADDITIVEKNOWLEDGEGREENPEEP_H
