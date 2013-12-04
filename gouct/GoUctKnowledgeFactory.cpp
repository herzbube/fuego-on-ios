//----------------------------------------------------------------------------
/** @file GoUctKnowledgeFactory.cpp
    See GoUctKnowledgeFactory.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "GoUctKnowledgeFactory.h"

#include "GoUctAdditiveKnowledge.h"
#include "GoUctAdditiveKnowledgeFuego.h"
#include "GoUctAdditiveKnowledgeGreenpeep.h"

//----------------------------------------------------------------------------
GoUctKnowledgeFactory::GoUctKnowledgeFactory(
    const GoUctPlayoutPolicyParam& param) :
    m_param(param)
{ }

GoUctKnowledgeFactory::~GoUctKnowledgeFactory()
{
}

GoUctAdditiveKnowledge* GoUctKnowledgeFactory::Create(const GoBoard& bd)
{
	KnowledgeType type = m_param.m_knowledgeType;
    
    switch(type)
    {
    case KNOWLEDGE_GREENPEEP:
    	return new GoUctAdditiveKnowledgeGreenpeep(bd);
    break;
    case KNOWLEDGE_RULEBASED:
    	return new GoUctAdditiveKnowledgeFuego(bd);
    break;
    default:
    	SG_ASSERT(false);
        return 0;
    }
}
//----------------------------------------------------------------------------
