#ifndef TESTDETAILS_H_
#define TESTDETAILS_H_

#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <vector>
#include "Attributes.h"
#include "LineInfo.h"
#include "xUnitTime.h"

namespace xUnitpp
{

class xUnitAssert;

struct TestDetails
{
    TestDetails();
    TestDetails(std::string &&name, int testInstance, std::string &&params, const std::string &suite,
        AttributeCollection &&attributes, Time::Duration timeLimit,
        std::string &&filename, int line);
    friend void swap(TestDetails &td0, TestDetails &td1);

    std::string FullName() const;

    int Id;
    int TestInstance;
    std::string Name;
    std::string Params;
    std::string Suite;
    AttributeCollection Attributes;
    Time::Duration TimeLimit;
    xUnitpp::LineInfo LineInfo;
};

}

#endif
