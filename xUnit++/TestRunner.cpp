#include "TestRunner.h"
#include <atomic>
#include <future>
#include <limits>
#include <mutex>
#include <random>
#include <vector>
#include "TestCollection.h"
#include "TestDetails.h"
#include "xUnitAssert.h"


// !!! temporary
#include <iostream>


namespace
{

class ActiveTests
{
public:
    struct TestInstance
    {
        TestInstance(const xUnitpp::TestDetails &testDetails, int id, int groupId, int groupSize, std::function<void()> test)
            : testDetails(testDetails)
            , id(id)
            , groupId(groupId)
            , groupSize(groupSize)
            , test(test)
        {
        }

        TestInstance(const TestInstance &other)
            : testDetails(other.testDetails)
            , id(other.id)
            , groupId(other.groupId)
            , groupSize(other.groupSize)
            , test(other.test)
        {
        }

        TestInstance(TestInstance &&other)
        {
            swap(*this, other);
        }

        TestInstance &operator =(TestInstance other)
        {
            swap(*this, other);
            return *this;
        }

        friend void swap(TestInstance &ti0, TestInstance &ti1)
        {
            using std::swap;

            swap(ti0.testDetails, ti1.testDetails);
            swap(ti0.id, ti1.id);
            swap(ti0.groupId, ti1.groupId);
            swap(ti0.groupSize, ti1.groupSize);
            swap(ti0.test, ti1.test);
        }

        xUnitpp::TestDetails testDetails;

        size_t id;
        size_t groupId;
        size_t groupSize;

        std::function<void()> test;
    };

    ActiveTests(const std::vector<xUnitpp::Fact> &facts, const std::vector<xUnitpp::Theory> &theories, const std::string &suite)
    {
        size_t id = 0;
        size_t groupId = 0;

        for (auto &fact : facts)
        {
            if (suite == "" || fact.TestDetails().Suite == suite)
            {
                mTests.emplace_back(TestInstance(fact.TestDetails(), ++id, ++groupId, 1, fact.Test()));
            }
        }

        for (auto &theorySet : theories)
        {
            if (suite == "" || theorySet.TestDetails().Suite == suite)
            {
                ++groupId;

                for (auto &theory : theorySet.Theories())
                {
                    mTests.emplace_back(TestInstance(theorySet.TestDetails(), ++id, groupId, theorySet.Theories().size(), theory));
                }
            }
        }

        std::shuffle(mTests.begin(), mTests.end(), std::default_random_engine(std::random_device()()));
    }

    std::vector<TestInstance>::iterator begin()
    {
        return mTests.begin();
    }

    std::vector<TestInstance>::iterator end()
    {
        return mTests.end();
    }

private:
    std::vector<TestInstance> mTests;
};

}

namespace xUnitpp
{

class TestRunner::Impl
{
public:
    Impl(std::function<void(const TestDetails &)> onTestStart,
         std::function<void(const TestDetails &, const std::string &)> onTestFailure,
         std::function<void(const TestDetails &, milliseconds)> onTestFinish,
         std::function<void(int, int, int, milliseconds)> onAllTestsComplete)
        : mOnTestStart(onTestStart)
        , mOnTestFailure(onTestFailure)
        , mOnTestFinish(onTestFinish)
        , mOnAllTestsComplete(onAllTestsComplete)
    {
    }

    void OnTestStart(const TestDetails &details)
    {
        std::lock_guard<std::mutex> guard(mStartMtx);
        mOnTestStart(details);
    }

    void OnTestFailure(const TestDetails &details, const std::string &message)
    {
        std::lock_guard<std::mutex> guard(mFailureMtx);
        mOnTestFailure(details, message);
    }

    void OnTestFinish(const TestDetails &details, milliseconds time)
    {
        std::lock_guard<std::mutex> guard(mFinishMtx);
        mOnTestFinish(details, time);
    }


    void OnAllTestsComplete(int total, int skipped, int failed, milliseconds totalTime)
    {
        mOnAllTestsComplete(total, skipped, failed, totalTime);
    }

private:
    std::function<void(const TestDetails &)> mOnTestStart;
    std::function<void(const TestDetails &, const std::string &)> mOnTestFailure;
    std::function<void(const TestDetails &, milliseconds)> mOnTestFinish;
    std::function<void(int, int, int, milliseconds)> mOnAllTestsComplete;

    std::mutex mStartMtx;
    std::mutex mFailureMtx;
    std::mutex mFinishMtx;
};

size_t RunAllTests(const std::string &suite, size_t maxTestRunTime, size_t maxConcurrent)
{
    return
        TestRunner([](const TestDetails &td) { std::cout << "starting test " << td.Name << std::endl; },
                   [](const TestDetails &td, const std::string &message) { std::cout << "test " << td.Name << " had failure: " << message << std::endl; },
                   [](const TestDetails &td, milliseconds time) { std::cout << "finished test " << td.Name << " in " << time.count() << " milliseconds" << std::endl; },
                   [](int failed, int skipped, int total, milliseconds time) { std::cout << total << " tests run, " << failed << "failure" << (failed != 1 ? "s" : "") << skipped << " skipped, taking " << time.count() << " milliseconds" << std::endl; })
            .RunTests(TestCollection::Facts(), TestCollection::Theories(), suite, maxTestRunTime, maxConcurrent);
}

TestRunner::TestRunner(std::function<void(const TestDetails &)> onTestStart,
                       std::function<void(const TestDetails &, const std::string &)> onTestFailure,
                       std::function<void(const TestDetails &, milliseconds)> onTestFinish,
                       std::function<void(int, int, int, milliseconds)> onAllTestsComplete)
    : mImpl(new Impl(onTestStart, onTestFailure, onTestFinish, onAllTestsComplete))
{
}

size_t TestRunner::RunTests(const std::vector<Fact> &facts, const std::vector<Theory> &theories, const std::string &suite, size_t maxTestRunTime, size_t maxConcurrent)
{
    auto timeStart = std::chrono::system_clock::now();

    ActiveTests activeTests(facts, theories, suite);

    if (maxConcurrent == 0)
    {
        maxConcurrent = std::numeric_limits<decltype(maxConcurrent)>::max();
    }

    class ThreadCounter
    {
    public:
        ThreadCounter(size_t maxThreads)
            : maxThreads(maxThreads)
            , activeThreads(0)
        {
        }

        void operator++()
        {
            std::unique_lock<std::mutex> lock(mtx);
            condition.wait(lock, [&]() { return activeThreads < maxThreads; });

            ++activeThreads;
        }

        void operator--()
        {
            std::lock_guard<std::mutex> guard(mtx);
            --activeThreads;
        }

    private:
        size_t maxThreads;
        size_t activeThreads;
        std::mutex mtx;
        std::condition_variable condition;
    } threadCounter(maxConcurrent);

    std::atomic<size_t> failedTests = 0;

    std::vector<std::future<void>> futures;
    for (auto &test : activeTests)
    {
        futures.push_back(std::async([&]()
            {
                struct CounterGuard
                {
                    CounterGuard(ThreadCounter &tc)
                        : tc(tc)
                    {
                        ++tc;
                    }

                    ~CounterGuard()
                    {
                        --tc;
                    }

                private:
                    ThreadCounter &tc;
                } counterGuard(threadCounter);

                decltype(Clock::now()) testStart;
                try
                {
                    mImpl->OnTestStart(test.testDetails);

                    testStart = Clock::now();
                    test.test();
                }
                catch (std::exception &e)
                {
                    mImpl->OnTestFailure(test.testDetails, e.what());
                    ++failedTests;
                }
                catch (...)
                {
                    mImpl->OnTestFailure(test.testDetails, "Unknown exception caught: test has crashed");
                    ++failedTests;
                }

                mImpl->OnTestFinish(test.testDetails, Clock::now() - testStart);
            }));
    }

    for (auto &test : futures)
    {
        test.get();
    }
    
    mImpl->OnAllTestsComplete(futures.size(), failedTests, 0, Clock::now() - timeStart);

    return failedTests;
}

}
