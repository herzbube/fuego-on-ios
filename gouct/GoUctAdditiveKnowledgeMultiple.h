//----------------------------------------------------------------------------
/** @file GoUctAdditiveKnowledgeMultiple.h 
    A class for applying more than one kind of additive knowledge.
    
    Also see GoAdditiveKnowledge.h
*/
//----------------------------------------------------------------------------

#ifndef GOUCT_ADDITIVE_KNOWLEDGE_MULTIPLE_H
#define GOUCT_ADDITIVE_KNOWLEDGE_MULTIPLE_H

#include "GoBoard.h"
#include "GoAdditiveKnowledge.h"
#include "GoUctPlayoutPolicy.h"
#include "SgVector.h"

//----------------------------------------------------------------------------

/** @todo a similar typedef could be used globally. */
typedef std::vector<SgUctMoveInfo> InfoVector;

//----------------------------------------------------------------------------

/** A container used for applying multiple compatible types of knowledge.
    The container is initially empty. Knowledge must be added using
    AddKnowledge before using this container. All added knowledge must
    share the same parameters PredictorType() and MinValue().
 
    The knowledge is additive in the sense that its combined value is
    added in the UCT child selection formula. However, the method
    of combining multiple types of knowledge can be chosen - @see
    GoUctKnowledgeCombinationType.
*/
class GoUctAdditiveKnowledgeMultiple: public GoAdditiveKnowledge
{
public:
    GoUctAdditiveKnowledgeMultiple(const GoBoard& bd,
                                   float minimum,
                                   GoUctKnowledgeCombinationType
                                   combinationType);
    
    ~GoUctAdditiveKnowledgeMultiple();
    
    /** GoUctAdditiveKnowledgeMultiple assumes ownership of added knowledge */
    void AddKnowledge(GoAdditiveKnowledge* knowledge);

    const GoBoard& Board() const;
    
	GoPredictorType PredictorType() const;

    /** The minimum value allowed by this predictor */
    float MinValue() const;

    void ProcessPosition(InfoVector& moves);

private:

    SgVector<GoAdditiveKnowledge*> m_additiveKnowledge;

    float m_minimum;

    GoUctKnowledgeCombinationType m_combinationType;

    const GoAdditiveKnowledge* FirstKnowledge() const;
    
    void InitPredictorValues(InfoVector& moves) const;
};

//----------------------------------------------------------------------------

inline float GoUctAdditiveKnowledgeMultiple::MinValue() const
{
	return m_minimum;
}

//----------------------------------------------------------------------------

#endif // GOUCT_ADDITIVE_KNOWLEDGE_MULTIPLE_H
