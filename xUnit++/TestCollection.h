#ifndef TESTCOLLECTION_H_
#define TESTCOLLECTION_H_

#include <functional>
#include <vector>
#include "Fact.h"
#include "Theory.h"


// !!! temporary
#include <iostream>
#include "xUnitAssert.h"

namespace xUnitpp
{

class TestCollection
{
    friend class Register;

public:
    class Register
    {
        // !!! someday, Visual Studio will understand variadic macros
        // when it does, fix this collection

        // !!! should use a macro system to automate this
        template<typename TFn, typename TArg0>
        static std::function<void()> TheoryHelper(TFn &&theory, std::tuple<TArg0> &&t)
        {
            return [=]() { return theory(std::get<0>(t)); };
        }

        template<typename TFn, typename TArg0, typename TArg1>
        static std::function<void()> TheoryHelper(TFn &&theory, std::tuple<TArg0, TArg1> &&t)
        {
            return [=]() { return theory(std::get<0>(t),
                                         std::get<1>(t)); };
        }

        template<typename TFn, typename TArg0, typename TArg1, typename TArg2>
        static std::function<void()> TheoryHelper(TFn &&theory, std::tuple<TArg0, TArg1, TArg2> &&t)
        {
            return [=]() { return theory(std::get<0>(t),
                                         std::get<1>(t),
                                         std::get<2>(t)); };
        }

        template<typename TFn, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
        static std::function<void()> TheoryHelper(TFn &&theory, std::tuple<TArg0, TArg1, TArg2, TArg3> &&t)
        {
            return [=]() { return theory(std::get<0>(t),
                                         std::get<1>(t),
                                         std::get<2>(t),
                                         std::get<3>(t)); };
        }

        template<typename TFn, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4>
        static std::function<void()> TheoryHelper(TFn &&theory, std::tuple<TArg0, TArg1, TArg2, TArg3, TArg4> &&t)
        {
            return [=]() { return theory(std::get<0>(t),
                                         std::get<1>(t),
                                         std::get<2>(t),
                                         std::get<3>(t),
                                         std::get<4>(t)); };
        }

    public:
        Register(const std::function<void()> &fn, const std::string &name, const std::string &suite, const std::string &filename, int line);

        template<typename TTheory, typename TTheoryData>
        Register(TTheory theory, TTheoryData theoryData, const std::string &name, const std::string &suite, const std::string &filename, int line)
        {
            std::vector<std::function<void()>> theorySet;

            for (auto t : theoryData())
            {
                theorySet.emplace_back(TheoryHelper(theory, std::move(t)));
            }
        
            TestCollection::Instance().mTheories.emplace_back(Theory(theorySet, name, suite, filename, line));
        }
    };

    // !!! temporary
    static void RunAllTests()
    {
        auto facts = 1;
        auto theories = 1;

        // !!! randomize
        for (auto fact : Instance().mFacts)
        {
            std::cout << "fact " << facts++ << " of " << Instance().mFacts.size() << std::endl;
            fact.Run();
        }

        // !!! randomize
        for (auto theorySet : Instance().mTheories)
        {
            std::cout << "theory " << theories++ << " of " << Instance().mTheories.size() << std::endl;

            auto instance = 1;
            auto count = theorySet.Theories().size();

            for (auto theory : theorySet.Theories())
            {
                std::cout << "theory instance " << instance++ << " of " << count << std::endl;
                theory();
            }
        }
    }

private:
    static TestCollection &Instance()
    {
        static TestCollection collection;
        return collection;
    }

    std::vector<Fact> mFacts;
    std::vector<Theory> mTheories;
};

}

#endif