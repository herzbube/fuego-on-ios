//----------------------------------------------------------------------------
/** @file SgRandom.cpp
    See SgRandom.h. */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgRandom.h"

#include <ctime>
#include <functional>
#include "SgDebug.h"

//----------------------------------------------------------------------------

SgRandom::GlobalData::GlobalData()
{
    m_seed = 0;
}

//----------------------------------------------------------------------------

SgRandom::SgRandom() : m_floatGenerator(m_generator)
{
    SetSeed();
    GetGlobalData().m_allGenerators.push_back(this);
}

SgRandom::~SgRandom()
{
    GetGlobalData().m_allGenerators.remove(this);
}

SgRandom& SgRandom::Global()
{
    static SgRandom s_globalGenerator;
    return s_globalGenerator;
}

SgRandom::GlobalData& SgRandom::GetGlobalData()
{
    static GlobalData s_data;
    return s_data;
}

int SgRandom::Seed()
{
    return GetGlobalData().m_seed;
}

void SgRandom::SetSeed()
{
    boost::mt19937::result_type seed = GetGlobalData().m_seed;
    if (seed == 0)
        return;
    m_generator.seed(seed);
}

void SgRandom::SetSeed(int seed)
{
    if (seed < 0)
    {
        GetGlobalData().m_seed = 0;
        return;
    }
    if (seed == 0)
        GetGlobalData().m_seed =
            static_cast<boost::mt19937::result_type>(std::time(0));
    else
        GetGlobalData().m_seed = seed;
    SgDebug() << "SgRandom::SetSeed: " << GetGlobalData().m_seed << '\n';
    // The original code used for_each combined with std::mem_fun. std::mem_fun
    // was removed in C++17. An easy replacement with the modern std::mem_fn
    // or std::bind could not be found, therefore this re-implementation.
    for (const auto& generator : GetGlobalData().m_allGenerators)
    {
        generator->SetSeed();
    }
    srand(GetGlobalData().m_seed);
}

//----------------------------------------------------------------------------
