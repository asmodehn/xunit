#ifndef STDOUTREPORTER_H_
#define STDOUTREPORTER_H_

#include "xUnit++/IOutput.h"

namespace xUnitpp
{

class StdOutReporter : public IOutput
{
public:
    StdOutReporter(bool verbose, bool veryVerbose);

    virtual void ReportStart(const TestDetails &) override;
    virtual void ReportEvent(const TestDetails &testDetails, const TestEvent &evt) override;
    virtual void ReportSkip(const TestDetails &testDetails, const std::string &reason) override;
    virtual void ReportFinish(const TestDetails &, Time::Duration) override;
    virtual void ReportAllTestsComplete(size_t testCount, size_t skipped, size_t failureCount, Time::Duration totalTime) override;

private:
    bool mVerbose;
    bool mVeryVerbose;
};

}

#endif
