//----------------------------------------------------------------------------
/** @file FeFeatureWeights.cpp  */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "FeFeatureWeights.h"

#include <iostream>
#include <sstream>
#include <string>
#include "FeBasicFeatures.h"
#include "FeData.h"
#include "SgDebug.h"
#include "SgException.h"

//----------------------------------------------------------------------------

// TODO get max index from computation
const std::size_t FeFeatureWeights::MAX_FEATURE_INDEX = 2200;

//----------------------------------------------------------------------------

FeFeatureWeights::FeFeatureWeights(size_t nuFeatures, size_t k)
    : m_nuFeatures(nuFeatures),
      m_k(k),
      m_minID(std::numeric_limits<size_t>::max()),
      m_maxID(std::numeric_limits<size_t>::min())
{
    SG_ASSERT(  nuFeatures == 0
             || nuFeatures == MAX_FEATURE_INDEX);
    m_w.resize(nuFeatures, 0);
    m_v.resize(k); // create empty vectors
    for (size_t i = 0; i < k; ++i)
        m_v[i].resize(nuFeatures, 0);
    SG_ASSERT(IsAllocated());
}

bool FeFeatureWeights::IsAllocated() const
{
    return m_w.size() == m_nuFeatures
    && m_w.capacity() == m_nuFeatures
    && m_v.size() == m_k
    && m_v.capacity() == m_k
    && ( (m_v.size() == 0)
        ||
        (
         m_v[0].size() == m_nuFeatures
         && m_v[0].capacity() == m_nuFeatures
         )
        );
}

FeFeatureWeights FeFeatureWeights::Read(std::istream& stream)
{
    std::string s;
    std::getline(stream, s, ':');
    if (s != "size")
        throw SgException("Expected \"size\" got " + s);
    size_t nuFeatures;
    stream >> nuFeatures;
    std::getline(stream, s, ':');
    if (s != ",k")
        throw SgException("Expected \",k\" got " + s);
    size_t k;
    stream >> k;
    SG_ASSERT(! stream.fail());

    FeFeatureWeights f(MAX_FEATURE_INDEX, k);
    for (size_t i = 0; i < nuFeatures; ++i)
    {
        size_t index;
        stream >> index;
        SG_ASSERT(! stream.fail());
        SG_ASSERT(index < MAX_FEATURE_INDEX);

        f.m_minID = std::min(f.m_minID, index);
        f.m_maxID = std::max(f.m_maxID, index);

        std::getline(stream, s, ',');
        SG_ASSERT(! stream.fail());
        float v;
        stream >> v;
        f.m_w[index] = v;
        SG_ASSERT(! stream.fail());
    }

    for (size_t i = 0; i < nuFeatures; ++i)
    {
        size_t index;
        stream >> index;
        SG_ASSERT(! stream.fail());
        SG_ASSERT(index < MAX_FEATURE_INDEX);
        for (size_t j = 0; j < k; ++j)
        {
            std::getline(stream, s, ',');
            SG_ASSERT(! stream.fail());
            float v;
            stream >> v;
            SG_ASSERT(! stream.fail());
            f.m_v[j][index] = v;
        }
        SG_ASSERT(! stream.fail());
    }
    return f;
}

FeFeatureWeights FeFeatureWeights::ReadDefaultWeights()
{
    try
    {
        std::istringstream in(gFeatureData);
        if (! in)
            throw SgException("Cannot open gFeatureData");
        return FeFeatureWeights::Read(in);
    }
    catch (const SgException& e)
    {
        SgDebug() << "Loading gFeatureData failed: " << e.what();
    }
    return FeFeatureWeights(0, 0);
}

//----------------------------------------------------------------------------

inline bool IsValidID(int id)
{
    return FeFeatures::IsBasicFeatureID(id)
        || FeFeatures::Is3x3PatternID(id);
}

std::ostream& operator<<(std::ostream& stream,
                         const FeFeatureWeights& w)
{
    stream << "FeFeatureWeights: Nu Features = " << w.m_nuFeatures
    << ", K = " << w.m_k
    << ", minID = " << w.m_minID
    << ", maxID = " << w.m_maxID
    << ", w[], v[] = \n";
    for (size_t i = 0; i < w.m_nuFeatures; ++i)
    {
        if (IsValidID(static_cast<int>(i)))
        {
            FeFeatures::WriteFeatureFromID(stream, static_cast<int>(i));
            stream << ": w[" << i << "] = "
                   << w.m_w[i] << "\nv = \n";
            for (size_t k = 0; k < w.m_k; ++k)
                stream << "v[" << k << "][" << i << "] = "
                       << w.m_v[k][i] << '\n';
        }
    }
    return stream;
}

