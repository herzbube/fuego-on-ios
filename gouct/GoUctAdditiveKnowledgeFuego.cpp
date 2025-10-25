//----------------------------------------------------------------------------
/** @file GoUctAdditiveKnowledgeFuego.cpp
    See GoUctAdditiveKnowledgeFuego.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "GoUctAdditiveKnowledgeFuego.h"

//----------------------------------------------------------------------------

/** @todo This is a tunable constant. */
const float GoUctAdditiveKnowledgeFuego::VALUE_MULTIPLIER = 4.0f;

//----------------------------------------------------------------------------

GoUctAdditiveKnowledgeFuego::GoUctAdditiveKnowledgeFuego(const GoBoard& bd)
    : GoUctAdditiveKnowledgeStdProb(bd)
{
    // Knowledge applies to all moves
    SetMoveRange(0, 10000); 
}

// @todo Assumes that SgMoveInfo has m_raveValue (and m_raveCount) populated
// by prior knowledge.
// This is a hack. Should call prior knowledge directly.
void 
GoUctAdditiveKnowledgeFuego::ProcessPosition(std::vector<SgUctMoveInfo>&
                                             moves)
{
    float sum = 0.0;
    // The original code use a variable length array. Variable length arrays
    // are not part of the C++ standard. Some compilers may support them, but
    // at least modern version of Clang nowadays generate a warning
    // ("Variable length arrays in C++ are a Clang extension").
    // Although the warning could be suppressed, it is better to not use the
    // language incorrectly. A good replacement for arrays is std::vector.
    std::vector<float> values(moves.size());
    for (size_t i = 0; i < moves.size(); ++i)
    {
        values[i] = exp(VALUE_MULTIPLIER * moves[i].m_raveValue);
        sum += values[i];
    }
    if (sum > 0.0)
        for (size_t i = 0; i < moves.size(); ++i)
            moves[i].m_predictorValue = values[i] / sum;
}

//----------------------------------------------------------------------------
