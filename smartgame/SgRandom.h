//----------------------------------------------------------------------------
/** @file SgRandom.h
    Random numbers. */
//----------------------------------------------------------------------------

#ifndef SG_RANDOM_H
#define SG_RANDOM_H

#include <algorithm>
#include <list>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>

//----------------------------------------------------------------------------

/** Random number generator.
    Uses a Mersenne Twister, because this is faster than std::rand() and
    game playing programs usually need faster random numbers more than
    high quality ones. All random generators are internally registered to
    make it possible to change the random seed for all of them.

    SgRandom is thread-safe (w.r.t. different instances) after construction
    (the constructor is not thread-safe, because it uses a global variable
    for registration). */
class SgRandom
{
public:
    SgRandom();

    ~SgRandom();

    /** Set random seed for all existing and future instances of SgRandom.
        @param seed The seed. If negative, no seed will be set. If zero, a
        non-deterministic random seed will be used (e.g. derived from the
        current time).
        Also calls std::srand()
        @note This function is not thread-safe. */
    static void SetSeed(int seed);

    /** Get random seed.
        See SetSeed(int) for the special meaning of zero and negative values. */
    static int Seed();

    /** Generate a float number in [0,range). */
    float Float(float range);

    /** Generate a float number in [0,1). */
    float Float_01();
    
    /** Get a random integer.
        Uses a fast random generator (the Mersenne Twister boost::mt19937),
        because in games and Monte Carlo simulations, speed is more important
        than quality. */
    unsigned int Int();

    /** Get a random integer in an interval.
        @param range The upper limit of the interval (exclusive)
        @pre range > 0
        @pre range <= SgRandom::Max()
        @return An integer in <tt> [0..range - 1]</tt> */
    int Int(int range);

    /** See SgRandom::Int(int) */
    std::size_t Int(std::size_t range);
    
    /** Get a small random integer in an interval.
        Uses only the lower 16 bits. Faster than SgRandom::Int(int) because it
        avoids the expensive modulo operation.
        @param range The upper limit of the interval (exclusive)
        @pre range > 0
        @pre range <= (1 << 16)
        @return An integer in <tt> [0..range - 1]</tt> */
    int SmallInt(int range);

    /** See SgRandom::SmallInt(int) */
    std::size_t SmallInt(std::size_t range);

    /** Get a random integer in [min, max - 1] */
    int Range(int min, int max);

    /** Maximum value. */
    unsigned int Max();

    /** convert percentage between 0 and 99 to a threshold for RandomEvent.
        Use as in following example:
        const unsigned int percent80 = PercentageThreshold(80); */
    unsigned int PercentageThreshold(int percentage);

    /** return true if random number SgRandom() <= threshold */
    bool RandomEvent(unsigned int threshold);

private:
    struct GlobalData
    {
        /** The random seed.
            Zero means not to set a random seed. */
        boost::mt19937::result_type m_seed;

        std::list<SgRandom*> m_allGenerators;

        GlobalData();
    };

    /** Return global data.
        Global data is stored as a static variable in this function to ensure
        that it is initialized at first call if SgRandom is used in global
        variables of other compilation units. */
    static GlobalData& GetGlobalData();

    boost::mt19937 m_generator;

    /*	Random number generator for Float() and Float_01(). 
    	Uses m_generator internally.
    	See http://www.boost.org/doc/libs/1_39_0/libs/random/
        random-distributions.html#uniform_01 
	*/
    boost::uniform_01<boost::mt19937, float> m_floatGenerator;

    void SetSeed();
    static void SetGlobalRandomSeed();
};

inline float SgRandom::Float_01()
{
    return m_floatGenerator();
}

inline float SgRandom::Float(float range)
{
    float v = m_floatGenerator() * range;
    SG_ASSERT(v <= range); 
    // @todo: should be < range? Worried about rounding issues.
    return v;
}

inline unsigned int SgRandom::Int()
{
    return m_generator();
}

inline int SgRandom::Int(int range)
{
    SG_ASSERT(range > 0);
    SG_ASSERT(static_cast<unsigned int>(range) <= SgRandom::Max());
    int i = Int() % range;
    SG_ASSERTRANGE(i, 0, range - 1);
    return i;
}

inline std::size_t SgRandom::Int(std::size_t range)
{
    SG_ASSERT(range <= SgRandom::Max());
    std::size_t i = Int() % range;
    SG_ASSERT(i < range);
    return i;
}

inline unsigned int SgRandom::Max()
{
    return m_generator.max();
}

inline unsigned int SgRandom::PercentageThreshold(int percentage)
{
    return (m_generator.max() / 100) * percentage;
}

inline bool SgRandom::RandomEvent(unsigned int threshold)
{
    return Int() <= threshold;
}

inline int SgRandom::Range(int min, int max)
{
    return min + Int(max - min);
}

inline int SgRandom::SmallInt(int range)
{
    SG_ASSERT(range > 0);
    SG_ASSERT(range <= (1 << 16));
    int i = ((Int() & 0xffff) * range) >> 16;
    SG_ASSERTRANGE(i, 0, range - 1);
    return i;
}

inline std::size_t SgRandom::SmallInt(std::size_t range)
{
    SG_ASSERT(range <= (1 << 16));
    std::size_t i = ((Int() & 0xffff) * range) >> 16;
    SG_ASSERT(i < range);
    return i;
}

#endif // SG_RANDOM_H
