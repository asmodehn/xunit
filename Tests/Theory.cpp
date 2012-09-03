#include <algorithm>
#include <string>
#include <tuple>
#include <vector>
#include "IOutput.h"
#include "xUnitTestRunner.h"
#include "xUnitTime.h"
#include "xUnit++.h"

using xUnitpp::Assert;

SUITE(Theory)
{

void TheoryUnderTest(int x)
{
    Assert.True(x == 0 || x == 1);
}

struct TheoryFixture
{
private:
    struct : xUnitpp::IOutput
    {
        virtual void ReportStart(const xUnitpp::TestDetails &) override
        {
        }

        virtual void ReportFailure(const xUnitpp::TestDetails &, const std::string &, const xUnitpp::LineInfo &) override
        {
        }

        virtual void ReportSkip(const xUnitpp::TestDetails &, const std::string &) override
        {
        }

        virtual void ReportFinish(const xUnitpp::TestDetails &, xUnitpp::Time::Duration) override
        {
        }

        virtual void ReportAllTestsComplete(size_t, size_t, size_t, xUnitpp::Time::Duration) override 
        {
        }
    } emptyReporter;

public:
    TheoryFixture()
        : localRunner(emptyReporter)
    {
    }

    template<typename TTheoryData>
    void Register(const std::string &name, TTheoryData &&theoryData)
    {
        xUnitpp::TestCollection::Register reg(collection, &TheoryUnderTest, theoryData, name, "Theory", attributes, -1, __FILE__, __LINE__);
    }

    void Run()
    {
        localRunner.RunTests([](const xUnitpp::TestDetails &) { return true; },
            collection.Tests(), xUnitpp::Time::Duration::zero(), 0);
    }

    template<typename TTheoryData>
    void RegisterAndRun(const std::string &name, TTheoryData &&theoryData)
    {
        Register(name, theoryData);

        Run();
    }

    xUnitpp::AttributeCollection attributes;
    xUnitpp::TestCollection collection;
    xUnitpp::TestRunner localRunner;
};

std::vector<std::tuple<int>> RawFunctionProvider()
{
    std::tuple<int> tuples[] =
    {
        std::make_tuple(0),
        std::make_tuple(1),
        std::make_tuple(2),
        std::make_tuple(3),
        std::make_tuple(0)
    };

    return std::vector<std::tuple<int>>(std::begin(tuples), std::end(tuples));
}

FACT_FIXTURE(TheoriesAcceptRawFunctions, TheoryFixture)
{
    RegisterAndRun("TheoriesAcceptRawFunctions", RawFunctionProvider);
}

FACT_FIXTURE(TheoriesAcceptStdFunction, TheoryFixture)
{
    std::function<std::vector<std::tuple<int>>()> provider = RawFunctionProvider;

    RegisterAndRun("TheoriesAcceptStdFunction", provider);
}

FACT_FIXTURE(TheoriesAcceptFunctors, TheoryFixture)
{
    struct
    {
        std::vector<std::tuple<int>> operator()() const
        {
            return RawFunctionProvider();
        }
    } functor;

    RegisterAndRun("TheoriesAcceptFunctors", functor);
}

FACT_FIXTURE(TheoriesGetAllDataPassedToThem, TheoryFixture)
{
    std::vector<int> dataProvided;

    // !!! this line appears to fix a somewhat random buffer overrun bug
    // I am not sure why. Investigate later.
    dataProvided.reserve(5);

    auto doTheory = [&](int x) { dataProvided.push_back(x); };
    xUnitpp::TestCollection::Register reg(collection, doTheory, RawFunctionProvider, "TheoriesGetAllDataPassedToThem", "Theory", attributes, -1, __FILE__, __LINE__);

    Run();

    Assert.Equal(2, std::count(dataProvided.begin(), dataProvided.end(), 0));
    Assert.Equal(1, std::count(dataProvided.begin(), dataProvided.end(), 1));
    Assert.Equal(1, std::count(dataProvided.begin(), dataProvided.end(), 2));
    Assert.Equal(1, std::count(dataProvided.begin(), dataProvided.end(), 3));
}

FACT_FIXTURE(TheoriesCanBeSkipped, TheoryFixture)
{
    attributes.insert(std::make_pair("Skip", "Testing skip."));

    auto doTheory = [](int) { Assert.Fail("Should not be run."); };

    xUnitpp::TestCollection::Register reg(collection, doTheory, RawFunctionProvider, "TheoriesGetAllDataPassedToThem", "Theory", attributes, -1, __FILE__, __LINE__);

    Run();
}

ATTRIBUTES(TheoriesCanHaveAttributes, ("Cats", "Meow"))
THEORY(TheoriesCanHaveAttributes, (int), RawFunctionProvider)
{
    for (const auto &test : xUnitpp::TestCollection::Instance().Tests())
    {
        if (test.TestDetails().Name == "TheoriesCanHaveAttributes")
        {
            auto it = test.TestDetails().Attributes.find("Cats");
            Assert.True(it != test.TestDetails().Attributes.end());
            Assert.True(it->second == "Meow");
            return;
        }
    }

    Assert.Fail("Could not find self in test list.");
}

std::vector<std::tuple<std::string, std::vector<std::tuple<int, std::string>>>> ComplexProvider()
{
    std::vector<std::tuple<std::string, std::vector<std::tuple<int, std::string>>>> result;

    std::vector<std::tuple<int, std::string>> internal_vector;
    internal_vector.push_back(std::make_tuple(0, std::string("xyz")));

    result.push_back(std::make_tuple(std::string("abcd"), internal_vector));

    return result;
}

THEORY(TheoriesCanAcceptComplexObjects, (const std::string &, const std::vector<std::tuple<int, std::string>> &), ComplexProvider)
{
    // just existing is good enough
}

}