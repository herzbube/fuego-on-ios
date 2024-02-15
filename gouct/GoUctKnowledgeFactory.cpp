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
#include "GoUctAdditiveKnowledgeMultiple.h"

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
    case KNOWLEDGE_NONE:
        return 0;
    case KNOWLEDGE_GREENPEEP:
    	return new GoUctAdditiveKnowledgeGreenpeep(bd);
    break;
    case KNOWLEDGE_RULEBASED:
    	return new GoUctAdditiveKnowledgeFuego(bd);
    break;
    case KNOWLEDGE_BOTH:
    {
        GoUctAdditiveKnowledgeFuego* f = new GoUctAdditiveKnowledgeFuego(bd);
        SgUctValue scale = f->Scale();
        SgUctValue minimum = f->Minimum();

        GoUctAdditiveKnowledgeMultiple* m =
        new GoUctAdditiveKnowledgeMultiple(bd, scale,  minimum,
                                           m_param.m_combinationType);
        m->AddKnowledge(f);
        m->AddKnowledge(
            new GoUctAdditiveKnowledgeGreenpeep(bd));
        return m;
    }
    break;
    default:
    	SG_ASSERT(false);
        return 0;
    }
}
//----------------------------------------------------------------------------
